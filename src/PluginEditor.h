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

    juce::Slider compThreshSlider;
    juce::Slider compRatioSlider;
    juce::Slider compAttackSlider;
    juce::Slider compReleaseSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compThreshAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compRatioAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compAttackAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compReleaseAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EasyEffectsAudioProcessorEditor)
};
