#include "PluginProcessor.h"
#include "PluginEditor.h"

EasyEffectsAudioProcessorEditor::EasyEffectsAudioProcessorEditor(EasyEffectsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customTheme);
    setSize(1000, 650);

    activeSlots = audioProcessor.getActiveSlots();

    // Header buttons
    addAndMakeVisible(savePresetBtn);
    addAndMakeVisible(loadPresetBtn);
    addAndMakeVisible(addEffectBtn);

    addEffectBtn.onClick = [this] { showAddEffectMenu(); };

    savePresetBtn.onClick = [this] {
        auto chooser = std::make_shared<juce::FileChooser>("Save Preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.xml");
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
        auto chooser = std::make_shared<juce::FileChooser>("Load Preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.xml");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc) {
                if (fc.getResults().isEmpty()) return;
                auto file = fc.getResult();
                juce::MemoryBlock destData;
                file.loadFileAsData(destData);
                audioProcessor.setStateInformation(destData.getData(), (int)destData.getSize());
                refreshSidebar();
            });
    };

    // Sidebar
    moduleList.setModel(this);
    moduleList.setRowHeight(38);
    moduleList.setColour(juce::ListBox::backgroundColourId, eeval::theme::bgSurface);
    addAndMakeVisible(moduleList);

    // Viewport
    viewport.setScrollBarsShown(true, false);
    viewport.setScrollBarThickness(8);
    addAndMakeVisible(viewport);

    // Footer
    globalFooterMeter = std::make_unique<eeval::ui::LevelMeterEditor>(audioProcessor.getLevelMeter());
    addAndMakeVisible(globalFooterMeter.get());

    // FFT
    fftAnalyzer = std::make_unique<eeval::ui::SpectrumAnalyzerEditor>(audioProcessor);
    addAndMakeVisible(fftAnalyzer.get());

    // Select first row
    int lastSelected = audioProcessor.getSelectedEditorIndex();
    if (lastSelected >= 0 && lastSelected < (int)activeSlots.size())
        moduleList.selectRow(lastSelected, false, true);
    else if (!activeSlots.empty())
        moduleList.selectRow(0, false, true);

    rebuildEditorView();
}

EasyEffectsAudioProcessorEditor::~EasyEffectsAudioProcessorEditor()
{
    currentEditor.reset();
    moduleList.setModel(nullptr);
    setLookAndFeel(nullptr);
}

void EasyEffectsAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(eeval::theme::bgBase);

    // Header
    g.setColour(eeval::theme::bgSurface);
    g.fillRect(0, 0, getWidth(), headerHeight);
    g.setColour(eeval::theme::borderSubtle);
    g.fillRect(0, headerHeight - 1, getWidth(), 1);
    g.setColour(eeval::theme::textPrimary);
    g.setFont(18.0f);
    g.drawText("Easy Effects", 15, 0, 200, headerHeight, juce::Justification::centredLeft);

    // Sidebar background
    int sidebarTop = headerHeight + fftHeight;
    int sidebarH = getHeight() - sidebarTop - footerHeight;
    g.setColour(eeval::theme::bgSurface);
    g.fillRect(0, sidebarTop, sidebarWidth, sidebarH);

    // Sidebar/content separator
    g.setColour(eeval::theme::borderSubtle);
    g.fillRect(sidebarWidth, sidebarTop + sidebarHeaderHeight, 1, sidebarH - sidebarHeaderHeight);

    // Footer
    g.setColour(eeval::theme::bgSurface);
    g.fillRect(0, getHeight() - footerHeight, getWidth(), footerHeight);
    g.setColour(eeval::theme::borderSubtle);
    g.fillRect(0, getHeight() - footerHeight, getWidth(), 1);
    g.setColour(eeval::theme::textSecondary);
    g.setFont(12.0f);
    g.drawText("48.0 kHz | EasyEffects Windows", 15, getHeight() - footerHeight, 250, footerHeight,
               juce::Justification::centredLeft);
}

void EasyEffectsAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Header
    auto header = area.removeFromTop(headerHeight);
    auto headerRight = header.removeFromRight(400);
    loadPresetBtn.setBounds(headerRight.removeFromRight(120).reduced(8, 10));
    savePresetBtn.setBounds(headerRight.removeFromRight(120).reduced(8, 10));

    // Footer
    auto footer = area.removeFromBottom(footerHeight);
    if (globalFooterMeter)
        globalFooterMeter->setBounds(footer.removeFromRight(350).reduced(5, 4));

    // FFT
    if (fftAnalyzer)
        fftAnalyzer->setBounds(area.removeFromTop(fftHeight).reduced(2, 2));

    // Sidebar area = add button + list
    auto sidebarArea = area.removeFromLeft(sidebarWidth);
    addEffectBtn.setBounds(sidebarArea.removeFromTop(sidebarHeaderHeight).reduced(6, 4));
    moduleList.setBounds(sidebarArea);

    // Main content
    viewport.setBounds(area);
    if (currentEditor != nullptr) {
        int contentHeight = juce::jmax(area.getHeight(), 450);
        currentEditor->setSize(area.getWidth() - 10, contentHeight);
    }
}

// --- ListBoxModel ---

int EasyEffectsAudioProcessorEditor::getNumRows() {
    return (int)activeSlots.size();
}

void EasyEffectsAudioProcessorEditor::paintListBoxItem(int, juce::Graphics&, int, int, bool) {
    // Custom painting handled by SidebarRowCustomComponent
}

juce::Component* EasyEffectsAudioProcessorEditor::refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existing) {
    if (rowNumber < 0 || rowNumber >= (int)activeSlots.size()) {
        delete existing;
        return nullptr;
    }

    auto* rowComp = dynamic_cast<eeval::ui::SidebarRowCustomComponent*>(existing);
    const auto& slot = activeSlots[(size_t)rowNumber];

    if (rowComp == nullptr) {
        rowComp = new eeval::ui::SidebarRowCustomComponent(
            audioProcessor.parameters,
            slot.slotIndex,
            slot.typeId,
            slot.displayName,
            rowNumber,
            // Remove callback
            [this](int slotIdx) {
                audioProcessor.removeEffect(slotIdx);
                refreshSidebar();
            },
            // Move up callback
            [this](int slotIdx) {
                audioProcessor.moveEffect(slotIdx, -1);
                refreshSidebar();
            },
            // Move down callback
            [this](int slotIdx) {
                audioProcessor.moveEffect(slotIdx, 1);
                refreshSidebar();
            }
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
    if (lastRowSelected >= 0 && lastRowSelected < (int)activeSlots.size()) {
        audioProcessor.setSelectedEditorIndex(lastRowSelected);
        rebuildEditorView();
    }
}

void EasyEffectsAudioProcessorEditor::refreshSidebar() {
    activeSlots = audioProcessor.getActiveSlots();
    moduleList.updateContent();
    moduleList.repaint();

    // Adjust selection
    int sel = audioProcessor.getSelectedEditorIndex();
    if (sel >= (int)activeSlots.size())
        sel = juce::jmax(0, (int)activeSlots.size() - 1);
    audioProcessor.setSelectedEditorIndex(sel);
    if (!activeSlots.empty())
        moduleList.selectRow(sel, false, true);

    rebuildEditorView();
}

void EasyEffectsAudioProcessorEditor::rebuildEditorView() {
    int index = audioProcessor.getSelectedEditorIndex();
    if (index < 0 || index >= (int)activeSlots.size()) {
        viewport.setViewedComponent(nullptr, false);
        currentEditor.reset();
        return;
    }

    const auto& slot = activeSlots[(size_t)index];
    std::string slotPrefix = eeval::EffectRegistry::slotPrefix(slot.slotIndex);

    viewport.setViewedComponent(nullptr, false);
    currentEditor = std::make_unique<eeval::ui::GenericModuleEditor>(
        audioProcessor.parameters, slot.slotIndex, slot.typeId, slot.displayName);

    viewport.setViewedComponent(currentEditor.get(), false);
    resized();
}

void EasyEffectsAudioProcessorEditor::showAddEffectMenu() {
    juce::PopupMenu menu;
    const auto& types = eeval::EffectRegistry::getEffectTypes();

    for (int i = 0; i < (int)types.size(); ++i) {
        menu.addItem(i + 1, types[(size_t)i].displayName);
    }

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&addEffectBtn),
        [this](int result) {
            if (result > 0) {
                const auto& types = eeval::EffectRegistry::getEffectTypes();
                int idx = result - 1;
                if (idx >= 0 && idx < (int)types.size()) {
                    int slotIdx = audioProcessor.addEffect(types[(size_t)idx].typeId);
                    if (slotIdx >= 0) {
                        refreshSidebar();
                        // Select the newly added effect
                        int newRow = (int)activeSlots.size() - 1;
                        audioProcessor.setSelectedEditorIndex(newRow);
                        moduleList.selectRow(newRow, false, true);
                        rebuildEditorView();
                    }
                }
            }
        });
}
