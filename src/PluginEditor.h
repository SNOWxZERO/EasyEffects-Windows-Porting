#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class EasyEffectsAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                       public juce::ListBoxModel
{
public:
    EasyEffectsAudioProcessorEditor (EasyEffectsAudioProcessor&);
    ~EasyEffectsAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    // ListBoxModel implementation
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    
    void updateEditorView(int row);
    void savePreset();
    void loadPreset();
    void autoSaveState();
    void autoLoadState();
    juce::File getAutoSaveFile() const;

private:
    EasyEffectsAudioProcessor& audioProcessor;
    
    juce::ListBox sidebarList;
    std::vector<std::string> loadedModules;
    
    // Top bar buttons
    juce::TextButton savePresetButton { "Save Preset" };
    juce::TextButton loadPresetButton { "Load Preset" };
    
    // Groups for switching UI views
    juce::Component gainEditorArea;
    juce::Component compEditorArea;

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
