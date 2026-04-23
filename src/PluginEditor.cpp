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
        auto* aw = new juce::AlertWindow("Save Global Preset", "Enter preset name:", juce::MessageBoxIconType::NoIcon);
        aw->addTextEditor("name", "", "Preset Name:");
        aw->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
        aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        
        aw->enterModalState(true, juce::ModalCallbackFunction::create([this, aw](int result) {
            if (result == 1) {
                auto name = aw->getTextEditorContents("name");
                if (name.isNotEmpty()) {
                    audioProcessor.getPresetManager().saveGlobalPreset(name.toStdString());
                }
            }
            delete aw;
        }));
    };

    loadPresetBtn.onClick = [this] {
        juce::PopupMenu menu;
        auto list = audioProcessor.getPresetManager().getGlobalPresetList();
        
        for (int i = 0; i < list.size(); ++i)
            menu.addItem(i + 1, list[i]);

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&loadPresetBtn),
            [this, list](int result) {
                if (result > 0) {
                    audioProcessor.getPresetManager().loadGlobalPreset(list[result - 1].toStdString());
                    refreshSidebar();
                }
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
    // Always delete and recreate — after reorder/remove, existing components
    // have stale slot indices, type IDs, and callbacks
    delete existing;

    if (rowNumber < 0 || rowNumber >= (int)activeSlots.size())
        return nullptr;

    const auto& slot = activeSlots[(size_t)rowNumber];

    auto* rowComp = new eeval::ui::SidebarRowCustomComponent(
        audioProcessor.parameters,
        slot.slotIndex,
        slot.typeId,
        slot.displayName,
        rowNumber,
        [this](int slotIdx) {
            audioProcessor.removeEffect(slotIdx);
            refreshSidebar();
        },
        [this](int slotIdx) {
            audioProcessor.moveEffect(slotIdx, -1);
            refreshSidebar();
        },
        [this](int slotIdx) {
            audioProcessor.moveEffect(slotIdx, 1);
            refreshSidebar();
        }
    );

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
    
    if (slot.typeId == "eq") {
        currentEditor = std::make_unique<eeval::ui::VisualEqualizerEditor>(
            audioProcessor.parameters, slot.slotIndex);
    } else {
        currentEditor = std::make_unique<eeval::ui::GenericModuleEditor>(
            audioProcessor.parameters, slot.slotIndex, slot.typeId, slot.displayName);
    }

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
