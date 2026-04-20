#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/Theme.h"
#include "ui/GenericModuleEditor.h"
#include "ui/LevelMeterEditor.h"
#include "ui/SpectrumAnalyzerEditor.h"
#include "ui/SidebarRowCustomComponent.h"
#include "dsp/EffectRegistry.h"

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
    juce::Component* refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    void selectedRowsChanged(int lastRowSelected) override;

    // Called by sidebar rows or externally to refresh
    void refreshSidebar();

private:
    void rebuildEditorView();
    void showAddEffectMenu();

    EasyEffectsAudioProcessor& audioProcessor;

    eeval::theme::CustomLookAndFeel customTheme;

    // Header
    juce::TextButton savePresetBtn { "Save Preset" };
    juce::TextButton loadPresetBtn { "Load Preset" };
    juce::TextButton addEffectBtn  { "+ Add Effect" };

    // Sidebar
    juce::ListBox moduleList;
    std::vector<EasyEffectsAudioProcessor::SlotInfo> activeSlots;

    // Global Panels
    std::unique_ptr<eeval::ui::LevelMeterEditor> globalFooterMeter;
    std::unique_ptr<eeval::ui::SpectrumAnalyzerEditor> fftAnalyzer;

    // Main Content
    juce::Viewport viewport;
    std::unique_ptr<juce::Component> currentEditor;

    // Layout constants
    static constexpr int headerHeight = 50;
    static constexpr int fftHeight = 120;
    static constexpr int footerHeight = 36;
    static constexpr int sidebarWidth = 190;
    static constexpr int sidebarHeaderHeight = 36;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EasyEffectsAudioProcessorEditor)
};
