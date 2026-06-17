#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class AllPassPhaseAudioProcessor;

class AllPassPhaseAudioProcessorEditor final : public juce::GenericAudioProcessorEditor {
  public:
	explicit AllPassPhaseAudioProcessorEditor(AllPassPhaseAudioProcessor& processor);
	~AllPassPhaseAudioProcessorEditor() override = default;

  private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AllPassPhaseAudioProcessorEditor)
};
