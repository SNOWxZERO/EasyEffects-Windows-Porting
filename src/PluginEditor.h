#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/Theme.h"
#include "ui/GenericModuleEditor.h"
#include "ui/LevelMeterEditor.h"

class EasyEffectsAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::ListBoxModel
{
public:
    EasyEffectsAudioProcessorEditor(EasyEffectsAudioProcessor&);
    ~EasyEffectsAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // ListBoxModel
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;

private:
    void rebuildEditorView();

    EasyEffectsAudioProcessor& audioProcessor;

    eeval::theme::CustomLookAndFeel customTheme;

    // Global Header
    juce::TextButton savePresetBtn { "Save Preset" };
    juce::TextButton loadPresetBtn { "Load Preset" };

    // Sidebar
    juce::ListBox moduleList;
    std::vector<std::string> displayNames;
    std::vector<std::string> moduleIds;

    // Main Content
    juce::Viewport viewport;
    std::unique_ptr<juce::Component> currentEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EasyEffectsAudioProcessorEditor)
};
