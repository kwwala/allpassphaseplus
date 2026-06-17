#pragma once

#include <array>

#include <juce_audio_processors/juce_audio_processors.h>

#include "AllPassFilter.h"

class AllPassPhaseAudioProcessor final : public juce::AudioProcessor {
  public:
	AllPassPhaseAudioProcessor();
	~AllPassPhaseAudioProcessor() override = default;

	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
	void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override { return true; }

	const juce::String getName() const override { return JucePlugin_Name; }
	bool acceptsMidi() const override { return false; }
	bool producesMidi() const override { return false; }
	bool isMidiEffect() const override { return false; }
	double getTailLengthSeconds() const override { return 0.0; }

	int getNumPrograms() override { return 1; }
	int getCurrentProgram() override { return 0; }
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	juce::AudioProcessorValueTreeState parameters;

	static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
	static float knobToFrequency(float normalisedValue);
	static float frequencyToKnob(float frequencyHz);

  private:
	static constexpr int maxFilters = 50;
	static constexpr int maxChannels = 2;
	static constexpr int deactivateAfterSamples = 16384;
	static constexpr float noiseFloor = 0.000007f;

	using FilterBank = std::array<AllPassFilter, maxFilters>;

	void setupFilters(float frequencyHz, float qValue);
	void zeroFilterState();
	void updateParameters();
	int getIterationCount() const;

	std::array<FilterBank, maxChannels> filters;
	juce::AudioBuffer<float> dryBuffer;

	std::atomic<float>* frequencyParameter = nullptr;
	std::atomic<float>* qParameter = nullptr;
	std::atomic<float>* iterationsParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;

	float currentFrequencyHz = 700.0f;
	float currentQ = 0.5f;
	float currentMix = 1.0f;
	float lastFrequencyKnob = frequencyToKnob(700.0f);
	int currentIterations = 25;
	int samplesSinceSilence = deactivateAfterSamples;
	double currentSampleRate = 44100.0;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AllPassPhaseAudioProcessor)
};
