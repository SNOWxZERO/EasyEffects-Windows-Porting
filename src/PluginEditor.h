#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class EasyEffectsAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    EasyEffectsAudioProcessorEditor(EasyEffectsAudioProcessor&);
    ~EasyEffectsAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    EasyEffectsAudioProcessor& audioProcessor;
    
    juce::Slider gainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EasyEffectsAudioProcessorEditor)
};
