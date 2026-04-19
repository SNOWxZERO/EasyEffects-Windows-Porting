#include "PluginProcessor.h"
#include "PluginEditor.h"

EasyEffectsAudioProcessorEditor::EasyEffectsAudioProcessorEditor(EasyEffectsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Apply global modern theme
    setLookAndFeel(&customTheme);
    
    setSize(1000, 650);

    displayNames = audioProcessor.getActiveEffectNames();
    moduleIds = audioProcessor.getActiveEffectIds();

    // Setup Header buttons
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
    moduleList.setRowHeight(36);
    moduleList.setColour(juce::ListBox::backgroundColourId, eeval::theme::bgSurface);
    addAndMakeVisible(moduleList);

    // Setup Viewport for main content
    viewport.setScrollBarsShown(true, false);
    viewport.setScrollBarThickness(8);
    addAndMakeVisible(viewport);

    // Setup Global Footer
    globalFooterMeter = std::make_unique<eeval::ui::LevelMeterEditor>(audioProcessor.getLevelMeter());
    addAndMakeVisible(globalFooterMeter.get());

    // Setup FFT Analyzer
    fftAnalyzer = std::make_unique<eeval::ui::SpectrumAnalyzerEditor>(audioProcessor);
    addAndMakeVisible(fftAnalyzer.get());

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
    // Main background
    g.fillAll(eeval::theme::bgBase);
    
    // Header background
    g.setColour(eeval::theme::bgSurface);
    g.fillRect(0, 0, getWidth(), headerHeight);

    // Header bottom border
    g.setColour(eeval::theme::borderSubtle);
    g.fillRect(0, headerHeight - 1, getWidth(), 1);
    
    // Draw logo/branding in header
    g.setColour(eeval::theme::textPrimary);
    g.setFont(18.0f);
    g.drawText("Easy Effects", 15, 0, 200, headerHeight, juce::Justification::centredLeft);

    // Sidebar background (explicit fill to ensure it's visible)
    g.setColour(eeval::theme::bgSurface);
    g.fillRect(0, headerHeight, sidebarWidth, getHeight() - headerHeight - footerHeight);

    // Sidebar right border (separator line)
    g.setColour(eeval::theme::borderSubtle);
    g.fillRect(sidebarWidth, headerHeight, 1, getHeight() - headerHeight - footerHeight - fftHeight);

    // Footer background
    g.setColour(eeval::theme::bgSurface);
    g.fillRect(0, getHeight() - footerHeight, getWidth(), footerHeight);

    // Footer top border
    g.setColour(eeval::theme::borderSubtle);
    g.fillRect(0, getHeight() - footerHeight, getWidth(), 1);

    // Footer status text
    g.setColour(eeval::theme::textSecondary);
    g.setFont(12.0f);
    g.drawText("48.0 kHz | EasyEffects Windows", 15, getHeight() - footerHeight, 250, footerHeight, juce::Justification::centredLeft);
}

void EasyEffectsAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // Header
    auto header = area.removeFromTop(headerHeight);
    // Buttons on the right side of header
    auto headerRight = header.removeFromRight(280);
    loadPresetBtn.setBounds(headerRight.removeFromRight(130).reduced(8, 10));
    savePresetBtn.setBounds(headerRight.removeFromRight(130).reduced(8, 10));
    
    // Footer
    auto footer = area.removeFromBottom(footerHeight);
    if (globalFooterMeter) {
        globalFooterMeter->setBounds(footer.removeFromRight(350).reduced(5, 4));
    }
    
    // FFT Analyzer (full width, below header)
    if (fftAnalyzer) {
        fftAnalyzer->setBounds(area.removeFromTop(fftHeight).reduced(2, 2));
    }

    // Sidebar
    moduleList.setBounds(area.removeFromLeft(sidebarWidth));
    
    // Main Content (remaining space)
    viewport.setBounds(area);
    if (currentEditor != nullptr) {
        int contentHeight = juce::jmax(area.getHeight(), 450); // Allow scroll for tall content
        currentEditor->setSize(area.getWidth() - 10, contentHeight);
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

void EasyEffectsAudioProcessorEditor::selectedRowsChanged(int lastRowSelected) {
    if (lastRowSelected >= 0 && lastRowSelected < (int)moduleIds.size()) {
        audioProcessor.setSelectedEditorIndex(lastRowSelected);
        rebuildEditorView();
    }
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
