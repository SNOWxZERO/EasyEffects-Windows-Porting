#include "PluginProcessor.h"
#include "PluginEditor.h"

EasyEffectsAudioProcessorEditor::EasyEffectsAudioProcessorEditor(EasyEffectsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customTheme);
    setSize(1000, 650);

    activeSlots = audioProcessor.getActiveSlots();

    // Header buttons
    addAndMakeVisible(presetBtn);
    addAndMakeVisible(bypassBtn);
    addAndMakeVisible(addEffectBtn);

    presetBtn.onClick = [this] { showPresetMenu(); };
    addEffectBtn.onClick = [this] { showAddEffectMenu(); };

    bypassBtn.setClickingTogglesState(true);
    bypassBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFE53935));
    bypassBtn.onClick = [this] {
        bool bypassed = bypassBtn.getToggleState();
        audioProcessor.setGlobalBypass(bypassed);
    };

    // Preset name label
    presetNameLabel.setFont(juce::Font(14.0f));
    presetNameLabel.setColour(juce::Label::textColourId, eeval::theme::textSecondary);
    presetNameLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(presetNameLabel);

    // Sidebar
    moduleList.setModel(this);
    moduleList.setRowHeight(38);
    moduleList.setColour(juce::ListBox::backgroundColourId, eeval::theme::bgSurface);
    addAndMakeVisible(moduleList);

    // Viewport
    viewport.setScrollBarsShown(true, false);
    viewport.setScrollBarThickness(8);
    addAndMakeVisible(viewport);

    // Select first row
    int lastSelected = audioProcessor.getSelectedEditorIndex();
    if (lastSelected >= 0 && lastSelected < (int)activeSlots.size())
        moduleList.selectRow(lastSelected, false, true);
    else if (!activeSlots.empty())
        moduleList.selectRow(0, false, true);

    rebuildEditorView();
    startTimerHz(10); // 10Hz for UI state updates
}

EasyEffectsAudioProcessorEditor::~EasyEffectsAudioProcessorEditor()
{
    stopTimer();
    currentEditor.reset();
    moduleList.setModel(nullptr);
    setLookAndFeel(nullptr);
}

void EasyEffectsAudioProcessorEditor::timerCallback()
{
    // Update preset name label
    auto& pm = audioProcessor.getPresetManager();
    juce::String label;
    if (pm.hasActivePreset()) {
        label = juce::String(pm.getActivePresetName());
        if (pm.isDirty()) label += " *";
    } else {
        label = "(No Preset)";
    }
    presetNameLabel.setText(label, juce::dontSendNotification);

    // Update bypass button visual
    bypassBtn.setToggleState(audioProcessor.getGlobalBypass(), juce::dontSendNotification);
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
    g.drawText("Easy Effects", 15, 0, 140, headerHeight, juce::Justification::centredLeft);

    // Sidebar background
    int sidebarTop = headerHeight;
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

    // Footer text - show sample rate and audio device info
    g.setColour(eeval::theme::textSecondary);
    g.setFont(12.0f);
    juce::String footerText = juce::String(audioProcessor.getSampleRate() / 1000.0, 1) + " kHz | EasyEffects Windows";
    g.drawText(footerText, 15, getHeight() - footerHeight, 350, footerHeight,
               juce::Justification::centredLeft);

    // Monitor hint in footer
    g.setColour(eeval::theme::textSecondary.withAlpha(0.6f));
    g.drawText(juce::CharPointer_UTF8("\xf0\x9f\x94\x8a Use Options button to set Input/Output devices"),
               getWidth() - 380, getHeight() - footerHeight, 370, footerHeight,
               juce::Justification::centredRight);
}

void EasyEffectsAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Header
    auto header = area.removeFromTop(headerHeight);
    header.removeFromLeft(150); // space for "Easy Effects" title

    // Header layout: [Preset Name] [Presets] [Bypass All] ... [right padding]
    auto headerRight = header.reduced(5, 10);
    bypassBtn.setBounds(headerRight.removeFromRight(100));
    headerRight.removeFromRight(8);
    presetBtn.setBounds(headerRight.removeFromRight(80));
    headerRight.removeFromRight(8);
    presetNameLabel.setBounds(headerRight);

    // Footer
    area.removeFromBottom(footerHeight);

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

    viewport.setViewedComponent(nullptr, false);
    
    if (slot.typeId == "eq") {
        currentEditor = std::make_unique<eeval::ui::VisualEqualizerEditor>(
            audioProcessor, audioProcessor.parameters, slot.slotIndex);
    } else {
        currentEditor = std::make_unique<eeval::ui::GenericModuleEditor>(
            audioProcessor.parameters, slot.slotIndex, slot.typeId, slot.displayName);
    }

    viewport.setViewedComponent(currentEditor.get(), false);
    resized();
}

void EasyEffectsAudioProcessorEditor::showPresetMenu() {
    auto& pm = audioProcessor.getPresetManager();
    juce::PopupMenu menu;

    // Save (overwrite current or prompt if no active preset)
    menu.addItem(1, "Save" + juce::String(pm.hasActivePreset() ? " (" + pm.getActivePresetName() + ")" : ""));
    menu.addItem(2, "Save As...");
    menu.addSeparator();

    // Load submenu
    auto list = pm.getGlobalPresetList();
    juce::PopupMenu loadMenu;
    for (int i = 0; i < list.size(); ++i)
        loadMenu.addItem(100 + i, list[i]);
    menu.addSubMenu("Load", loadMenu, list.size() > 0);

    // Delete submenu
    juce::PopupMenu deleteMenu;
    for (int i = 0; i < list.size(); ++i)
        deleteMenu.addItem(200 + i, list[i]);
    menu.addSubMenu("Delete", deleteMenu, list.size() > 0);

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&presetBtn),
        [this, &pm, list](int result) {
            if (result == 1) {
                // Save: overwrite active preset, or prompt if none
                if (pm.hasActivePreset()) {
                    pm.saveGlobalPreset(pm.getActivePresetName());
                } else {
                    // Prompt for name (same as Save As)
                    auto* aw = new juce::AlertWindow("Save Preset", "Enter preset name:", juce::MessageBoxIconType::NoIcon);
                    aw->addTextEditor("name", "", "Name:");
                    aw->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
                    aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
                    aw->enterModalState(true, juce::ModalCallbackFunction::create([this, aw](int res) {
                        if (res == 1) {
                            auto name = aw->getTextEditorContents("name").toStdString();
                            if (!name.empty())
                                audioProcessor.getPresetManager().saveGlobalPreset(name);
                        }
                        delete aw;
                    }));
                }
            } else if (result == 2) {
                // Save As: always prompt
                auto* aw = new juce::AlertWindow("Save Preset As", "Enter new preset name:", juce::MessageBoxIconType::NoIcon);
                aw->addTextEditor("name", "", "Name:");
                aw->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
                aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
                aw->enterModalState(true, juce::ModalCallbackFunction::create([this, aw](int res) {
                    if (res == 1) {
                        auto name = aw->getTextEditorContents("name").toStdString();
                        if (!name.empty())
                            audioProcessor.getPresetManager().saveGlobalPreset(name);
                    }
                    delete aw;
                }));
            } else if (result >= 200 && result < 300) {
                // Delete with confirmation
                auto name = list[result - 200].toStdString();
                auto opts = juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::WarningIcon)
                    .withTitle("Delete Preset")
                    .withMessage("Delete \"" + juce::String(name) + "\"?")
                    .withButton("Delete")
                    .withButton("Cancel");
                juce::AlertWindow::showAsync(opts, [this, name](int r) {
                    if (r == 1) {
                        audioProcessor.getPresetManager().deleteGlobalPreset(name);
                    }
                });
            } else if (result >= 100 && result < 200) {
                // Load
                pm.loadGlobalPreset(list[result - 100].toStdString());
                refreshSidebar();
            }
        });
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
                        int newRow = (int)activeSlots.size() - 1;
                        audioProcessor.setSelectedEditorIndex(newRow);
                        moduleList.selectRow(newRow, false, true);
                        rebuildEditorView();
                    }
                }
            }
        });
}
