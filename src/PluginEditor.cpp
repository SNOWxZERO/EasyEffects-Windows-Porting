#include "PluginProcessor.h"
#include "PluginEditor.h"

EasyEffectsAudioProcessorEditor::EasyEffectsAudioProcessorEditor(EasyEffectsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Apply global modern theme
    setLookAndFeel(&customTheme);
    
    setSize(900, 600);

    displayNames = audioProcessor.getActiveEffectNames();
    moduleIds = audioProcessor.getActiveEffectIds();

    // Setup Header
    addAndMakeVisible(savePresetBtn);
    addAndMakeVisible(loadPresetBtn);
    
    savePresetBtn.onClick = [this] {
        auto chooser = std::make_shared<juce::FileChooser>("Save Preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.xml");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc) {
                if (fc.getResults().isEmpty()) return;
                auto file = fc.getResult();
                juce::MemoryBlock destData;
                audioProcessor.getStateInformation(destData);
                file.replaceWithData(destData.getData(), destData.getSize());
            });
    };
    
    loadPresetBtn.onClick = [this] {
        auto chooser = std::make_shared<juce::FileChooser>("Load Preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.xml");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc) {
                if (fc.getResults().isEmpty()) return;
                auto file = fc.getResult();
                juce::MemoryBlock destData;
                file.loadFileAsData(destData);
                audioProcessor.setStateInformation(destData.getData(), (int)destData.getSize());
                
                // Restore UI state
                moduleList.selectRow(audioProcessor.getSelectedEditorIndex(), true, true);
            });
    };

    // Setup Sidebar
    moduleList.setModel(this);
    moduleList.setRowHeight(40);
    addAndMakeVisible(moduleList);

    // Setup Viewport for main content
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);

    int lastSelected = audioProcessor.getSelectedEditorIndex();
    if (lastSelected >= 0 && lastSelected < (int)displayNames.size()) {
        moduleList.selectRow(lastSelected, false, true);
    } else {
        moduleList.selectRow(0, false, true);
    }
    
    // Explicitly build the view because selectRow does not trigger listBoxItemClicked natively
    rebuildEditorView();
}

EasyEffectsAudioProcessorEditor::~EasyEffectsAudioProcessorEditor()
{
    // Always tear down UI state properly
    currentEditor.reset();
    moduleList.setModel(nullptr);
    setLookAndFeel(nullptr);
}

void EasyEffectsAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(eeval::theme::bgBase);
    
    // Draw header background
    g.setColour(eeval::theme::bgSurface);
    g.fillRect(0, 0, getWidth(), 60);
    
    // Draw logo/branding
    g.setColour(eeval::theme::accentPrimary);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("EasyEffects Windows", 20, 0, 300, 60, juce::Justification::centredLeft);
}

void EasyEffectsAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // Header
    auto header = area.removeFromTop(40);
    loadPresetBtn.setBounds(header.removeFromRight(120).reduced(5));
    savePresetBtn.setBounds(header.removeFromRight(120).reduced(5));
    
    // Global Footer
    auto footer = area.removeFromBottom(40);
    if (globalFooterMeter) {
        globalFooterMeter->setBounds(footer.removeFromRight(300));
    }
    
    // Global Spectrum Analyzer (Top)
    fftPlaceholder.setBounds(area.removeFromTop(150).reduced(5));

    // Sidebar
    moduleList.setBounds(area.removeFromLeft(200));
    
    // Main Content
    viewport.setBounds(area);
    if (currentEditor != nullptr) {
        currentEditor->setSize(area.getWidth() - 20, 800); // Fixed high height to allow scroll
    }
}

// ListBoxModel Overrides
int EasyEffectsAudioProcessorEditor::getNumRows() {
    return (int)displayNames.size();
}

void EasyEffectsAudioProcessorEditor::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) {
    juce::ignoreUnused(rowNumber, g, width, height, rowIsSelected);
    // Custom painting handled entirely within SidebarRowCustomComponent
}

juce::Component* EasyEffectsAudioProcessorEditor::refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) {
    if (rowNumber < 0 || rowNumber >= (int)displayNames.size()) {
        delete existingComponentToUpdate;
        return nullptr;
    }

    auto* rowComp = dynamic_cast<eeval::ui::SidebarRowCustomComponent*>(existingComponentToUpdate);

    if (rowComp == nullptr) {
        rowComp = new eeval::ui::SidebarRowCustomComponent(
            audioProcessor.parameters, 
            moduleIds[(size_t)rowNumber], 
            displayNames[(size_t)rowNumber], 
            rowNumber
        );
    }

    rowComp->update(isRowSelected);
    return rowComp;
}

void EasyEffectsAudioProcessorEditor::listBoxItemClicked(int row, const juce::MouseEvent&) {
    audioProcessor.setSelectedEditorIndex(row);
    rebuildEditorView();
}

void EasyEffectsAudioProcessorEditor::rebuildEditorView() {
    int index = audioProcessor.getSelectedEditorIndex();
    if (index < 0 || index >= (int)moduleIds.size()) return;
    
    std::string modId = moduleIds[index];
    std::string modName = displayNames[index];
    
    // Hot-swap the editor component safely
    viewport.setViewedComponent(nullptr, false);
    
    if (modId == "meter") {
        currentEditor = std::make_unique<eeval::ui::LevelMeterEditor>(audioProcessor.getLevelMeter());
    } else {
        currentEditor = std::make_unique<eeval::ui::GenericModuleEditor>(audioProcessor.parameters, modId, modName);
    }
    
    viewport.setViewedComponent(currentEditor.get(), false);
    resized(); // Recalculate layout sizes
}
