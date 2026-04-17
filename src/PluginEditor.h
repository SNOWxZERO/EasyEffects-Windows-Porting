#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// Phase 3B: Temporary minimal editor — DSP-only phase.
// Full UI rebuild comes in Phase 4B after all DSP modules are stable.
class EasyEffectsAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    EasyEffectsAudioProcessorEditor(EasyEffectsAudioProcessor&);
    ~EasyEffectsAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    EasyEffectsAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EasyEffectsAudioProcessorEditor)
};
