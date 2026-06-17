#include "PluginProcessor.h"

#include <cmath>
#include <memory>

#include "PluginEditor.h"

namespace {
constexpr float defaultFrequencyHz = 700.0f;
constexpr float defaultQ = 0.70710677f;
constexpr float defaultMix = 1.0f;
constexpr int defaultIterations = 25;

juce::String percentageToString(float value, int) {
	return juce::String(juce::roundToInt(value * 100.0f));
}

float percentageFromString(const juce::String& text) {
	return text.getFloatValue() / 100.0f;
}
} // namespace

AllPassPhaseAudioProcessor::AllPassPhaseAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("AllPassPhase"), createParameterLayout()) {
	frequencyParameter = parameters.getRawParameterValue("Frequency");
	qParameter = parameters.getRawParameterValue("Q");
	iterationsParameter = parameters.getRawParameterValue("Iterations");
	mixParameter = parameters.getRawParameterValue("Mix");

	updateParameters();
}

juce::AudioProcessorValueTreeState::ParameterLayout AllPassPhaseAudioProcessor::createParameterLayout() {
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	auto frequencyRange = juce::NormalisableRange<float>(
	    knobToFrequency(0.0f), knobToFrequency(1.0f),
	    [](float rangeStart, float rangeEnd, float normalised) {
		    juce::ignoreUnused(rangeStart, rangeEnd);
		    return knobToFrequency(normalised);
	    },
	    [](float rangeStart, float rangeEnd, float value) {
		    juce::ignoreUnused(rangeStart, rangeEnd);
		    return frequencyToKnob(value);
	    },
	    [](float rangeStart, float rangeEnd, float value) {
		    return static_cast<float>(juce::roundToInt(juce::jlimit(rangeStart, rangeEnd, value)));
	    });

	layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Frequency", 1), "Frequency",
	                                                       frequencyRange, defaultFrequencyHz,
	                                                       juce::AudioParameterFloatAttributes().withLabel("Hz")));

	layout.add(std::make_unique<juce::AudioParameterFloat>(
	    juce::ParameterID("Q", 1), "Q", juce::NormalisableRange<float>(0.0f, std::sqrt(2.0f), 0.0f, 1.0f), defaultQ));

	layout.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID("Iterations", 1), "Intensity", 0, maxFilters,
	                                                     defaultIterations,
	                                                     juce::AudioParameterIntAttributes().withLabel("iterations")));

	layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Mix", 1), "Mix",
	                                                       juce::NormalisableRange<float>(0.0f, 1.0f), defaultMix,
	                                                       juce::AudioParameterFloatAttributes()
	                                                           .withLabel("%")
	                                                           .withStringFromValueFunction(percentageToString)
	                                                           .withValueFromStringFunction(percentageFromString)));

	return layout;
}

float AllPassPhaseAudioProcessor::knobToFrequency(float normalisedValue) {
	const auto clampedValue = juce::jlimit(0.0f, 1.0f, normalisedValue);
	return std::floor(std::exp((16.0f + clampedValue * 100.0f * 1.20103f) * std::log(1.059f)) * 8.17742f);
}

float AllPassPhaseAudioProcessor::frequencyToKnob(float frequencyHz) {
	const auto clampedFrequency = juce::jlimit(knobToFrequency(0.0f), knobToFrequency(1.0f), frequencyHz);
	const auto value = (std::log(clampedFrequency / 8.17742f) / std::log(1.059f) - 16.0f) / (100.0f * 1.20103f);
	return juce::jlimit(0.0f, 1.0f, value);
}

void AllPassPhaseAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
	currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
	dryBuffer.setSize(maxChannels, juce::jmax(1, samplesPerBlock), false, false, true);
	zeroFilterState();
	samplesSinceSilence = deactivateAfterSamples;
	updateParameters();
}

void AllPassPhaseAudioProcessor::releaseResources() {
	dryBuffer.setSize(0, 0);
	zeroFilterState();
	samplesSinceSilence = deactivateAfterSamples;
}

bool AllPassPhaseAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
	const auto& input = layouts.getMainInputChannelSet();
	const auto& output = layouts.getMainOutputChannelSet();

	if (input != output)
		return false;

	return output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
}

void AllPassPhaseAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
	juce::ignoreUnused(midiMessages);
	juce::ScopedNoDenormals noDenormals;

	const auto totalInputChannels = getTotalNumInputChannels();
	const auto totalOutputChannels = getTotalNumOutputChannels();
	const auto numChannels = juce::jmin(totalInputChannels, totalOutputChannels, maxChannels);
	const auto numSamples = buffer.getNumSamples();

	for (auto channel = totalInputChannels; channel < totalOutputChannels; ++channel)
		buffer.clear(channel, 0, numSamples);

	if (numSamples <= 0 || numChannels <= 0)
		return;

	updateParameters();

	if (currentIterations == 0 || currentMix <= 0.0f) {
		return;
	}

	jassert(numSamples <= dryBuffer.getNumSamples());
	if (numSamples > dryBuffer.getNumSamples()) {
		return;
	}

	for (int channel = 0; channel < numChannels; ++channel)
		dryBuffer.copyFrom(channel, 0, buffer, channel, 0, numSamples);

	for (int sample = 0; sample < numSamples; ++sample) {
		for (int channel = 0; channel < numChannels; ++channel) {
			if (std::abs(dryBuffer.getSample(channel, sample)) >= noiseFloor) {
				samplesSinceSilence = 0;
				break;
			}
		}
	}

	if (samplesSinceSilence < deactivateAfterSamples) {
		for (int channel = 0; channel < numChannels; ++channel) {
			auto* channelData = buffer.getWritePointer(channel);
			auto& channelFilters = filters[static_cast<size_t>(channel)];

			for (int filterIndex = 0; filterIndex < currentIterations; ++filterIndex)
				channelFilters[static_cast<size_t>(filterIndex)].processBlock(channelData, channelData, numSamples);
		}
	}

	const auto dryMix = 1.0f - currentMix;

	for (int sample = 0; sample < numSamples; ++sample) {
		bool hasSignal = false;

		for (int channel = 0; channel < numChannels; ++channel) {
			auto* channelData = buffer.getWritePointer(channel);
			const auto wetSample = channelData[sample];
			const auto drySample = dryBuffer.getSample(channel, sample);

			if (std::abs(wetSample) >= noiseFloor)
				hasSignal = true;

			channelData[sample] = wetSample * currentMix + drySample * dryMix;
		}

		if (hasSignal) {
			samplesSinceSilence = 0;
		} else if (samplesSinceSilence < 32768) {
			++samplesSinceSilence;
		}
	}
}

juce::AudioProcessorEditor* AllPassPhaseAudioProcessor::createEditor() {
	return new AllPassPhaseAudioProcessorEditor(*this);
}

void AllPassPhaseAudioProcessor::setCurrentProgram(int index) {
	juce::ignoreUnused(index);
}

const juce::String AllPassPhaseAudioProcessor::getProgramName(int index) {
	juce::ignoreUnused(index);
	return "Init";
}

void AllPassPhaseAudioProcessor::changeProgramName(int index, const juce::String& newName) {
	juce::ignoreUnused(index, newName);
}

void AllPassPhaseAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
	if (auto state = parameters.copyState(); auto xml = state.createXml())
		copyXmlToBinary(*xml, destData);
}

void AllPassPhaseAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
	if (auto xml = getXmlFromBinary(data, sizeInBytes)) {
		if (xml->hasTagName(parameters.state.getType())) {
			parameters.replaceState(juce::ValueTree::fromXml(*xml));
			updateParameters();
		}
	}
}

void AllPassPhaseAudioProcessor::setupFilters(float frequencyHz, float qValue) {
	auto clampedFrequency = frequencyHz;
	const auto sampleRate = currentSampleRate > 0.0 ? static_cast<float>(currentSampleRate) : 44100.0f;
	const auto nyquist = sampleRate * 0.5f;

	if (clampedFrequency >= nyquist)
		clampedFrequency = sampleRate * 0.49f;

	const auto clampedQ = juce::jmax(0.005f, qValue);
	const auto frequencyKnob = frequencyToKnob(clampedFrequency);
	const auto resetFilterState =
	    std::abs(frequencyKnob - lastFrequencyKnob) > frequencyKnob / 10.0f && clampedFrequency < 500.0f;

	for (auto& channelFilters : filters) {
		channelFilters[0].setup(clampedFrequency, sampleRate, clampedQ);

		if (resetFilterState)
			channelFilters[0].zeroBuffers();

		for (int i = 1; i < maxFilters; ++i) {
			channelFilters[static_cast<size_t>(i)].copyCoefficientsFrom(channelFilters[0]);

			if (resetFilterState)
				channelFilters[static_cast<size_t>(i)].zeroBuffers();
		}
	}

	lastFrequencyKnob = frequencyKnob;
	currentFrequencyHz = clampedFrequency;
	currentQ = clampedQ;
}

void AllPassPhaseAudioProcessor::zeroFilterState() {
	for (auto& channelFilters : filters) {
		for (auto& filter : channelFilters)
			filter.zeroBuffers();
	}
}

void AllPassPhaseAudioProcessor::updateParameters() {
	const auto nextFrequency = frequencyParameter != nullptr ? frequencyParameter->load() : defaultFrequencyHz;
	const auto nextQ = juce::jmax(0.005f, qParameter != nullptr ? qParameter->load() : defaultQ);
	const auto nextIterations =
	    iterationsParameter != nullptr ? juce::roundToInt(iterationsParameter->load()) : defaultIterations;
	const auto nextMix = mixParameter != nullptr ? mixParameter->load() : defaultMix;

	if (!juce::approximatelyEqual(nextFrequency, currentFrequencyHz) || !juce::approximatelyEqual(nextQ, currentQ))
		setupFilters(nextFrequency, nextQ);

	currentIterations = juce::jlimit(0, maxFilters, nextIterations);
	currentMix = juce::jlimit(0.0f, 1.0f, nextMix);
}

int AllPassPhaseAudioProcessor::getIterationCount() const {
	return currentIterations;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
	return new AllPassPhaseAudioProcessor();
}
