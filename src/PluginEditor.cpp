#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ui/ConvolverEditor.h"

// Access JUCE standalone holder for device management
#if JucePlugin_Build_Standalone
 #include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

EasyEffectsAudioProcessorEditor::EasyEffectsAudioProcessorEditor(EasyEffectsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customTheme);
    setSize(1000, 650);

    activeSlots = audioProcessor.getActiveSlots();

    // Header buttons
    addAndMakeVisible(presetBtn);
    addAndMakeVisible(bypassBtn);
    addAndMakeVisible(monitorBtn);
    addAndMakeVisible(addEffectBtn);

    presetBtn.onClick = [this] { showPresetMenu(); };
    addEffectBtn.onClick = [this] { showAddEffectMenu(); };

    bypassBtn.setClickingTogglesState(true);
    bypassBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFE53935));
    bypassBtn.onClick = [this] {
        audioProcessor.setGlobalBypass(bypassBtn.getToggleState());
    };

    // Monitor button - toggles output device
    monitorBtn.setClickingTogglesState(true);
    monitorBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF43A047));
    monitorBtn.onClick = [this] { toggleMonitor(); };

    // Load saved monitor device from settings
#if JucePlugin_Build_Standalone
    if (auto* holder = juce::StandalonePluginHolder::getInstance()) {
        if (auto* props = holder->settings.get())
            monitorDeviceName = props->getValue("monitorDevice", "");
    }
#endif

    // Preset name label
    presetNameLabel.setFont(juce::Font(14.0f));
    presetNameLabel.setColour(juce::Label::textColourId, eeval::theme::textSecondary);
    presetNameLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(presetNameLabel);

    // FFT Analyzer
    fftAnalyzer = std::make_unique<eeval::ui::SpectrumAnalyzerEditor>(audioProcessor);
    addAndMakeVisible(fftAnalyzer.get());

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
    startTimerHz(10);
}

EasyEffectsAudioProcessorEditor::~EasyEffectsAudioProcessorEditor()
{
    // If monitoring, restore original device before closing
    if (isMonitoring)
        toggleMonitor();

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

    // Sync bypass button
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
    juce::String footerText = juce::String(audioProcessor.getSampleRate() / 1000.0, 1) + " kHz | EasyEffects Windows";

    // Show current output device info
#if JucePlugin_Build_Standalone
    if (auto* holder = juce::StandalonePluginHolder::getInstance()) {
        if (auto* device = holder->deviceManager.getCurrentAudioDevice()) {
            footerText = juce::String(device->getCurrentSampleRate() / 1000.0, 1) + " kHz";
            footerText += " | Out: " + device->getOutputChannelNames()[0];
        }
    }
#endif

    g.drawText(footerText, 15, getHeight() - footerHeight, getWidth() - 30, footerHeight,
               juce::Justification::centredLeft);

    // Monitor device indicator on the right
    if (isMonitoring) {
        g.setColour(juce::Colour(0xFF43A047));
        g.drawText(">> Monitoring: " + monitorDeviceName,
                   getWidth() / 2, getHeight() - footerHeight, getWidth() / 2 - 15, footerHeight,
                   juce::Justification::centredRight);
    }
}

void EasyEffectsAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Header
    auto header = area.removeFromTop(headerHeight);
    header.removeFromLeft(150);

    auto headerRight = header.reduced(5, 10);
    monitorBtn.setBounds(headerRight.removeFromRight(85));
    headerRight.removeFromRight(5);
    bypassBtn.setBounds(headerRight.removeFromRight(90));
    headerRight.removeFromRight(5);
    presetBtn.setBounds(headerRight.removeFromRight(75));
    headerRight.removeFromRight(5);
    presetNameLabel.setBounds(headerRight);

    // Footer
    auto footer = area.removeFromBottom(footerHeight);

    // FFT Analyzer
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

// --- Monitor Toggle ---

void EasyEffectsAudioProcessorEditor::toggleMonitor()
{
#if JucePlugin_Build_Standalone
    auto* holder = juce::StandalonePluginHolder::getInstance();
    if (!holder) return;

    auto& dm = holder->deviceManager;

    if (!isMonitoring) {
        // Check if monitor device is configured
        if (monitorDeviceName.isEmpty()) {
            // Prompt user to choose a monitor device
            juce::PopupMenu menu;
            menu.addSectionHeader("Choose Monitor Output Device:");

            auto* currentDevice = dm.getCurrentAudioDevice();
            auto* deviceType = currentDevice ? dm.getCurrentDeviceTypeObject() : nullptr;
            
            juce::StringArray outputDevices;
            if (deviceType)
                outputDevices = deviceType->getDeviceNames(false);

            for (int i = 0; i < outputDevices.size(); ++i)
                menu.addItem(i + 1, outputDevices[i]);

            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&monitorBtn),
                [this, outputDevices](int result) {
                    if (result > 0) {
                        monitorDeviceName = outputDevices[result - 1];
                        // Save to settings
                    #if JucePlugin_Build_Standalone
                        if (auto* h = juce::StandalonePluginHolder::getInstance())
                            if (auto* props = h->settings.get())
                                props->setValue("monitorDevice", monitorDeviceName);
                    #endif
                        // Now actually toggle
                        monitorBtn.setToggleState(true, juce::dontSendNotification);
                        toggleMonitor();
                    } else {
                        monitorBtn.setToggleState(false, juce::dontSendNotification);
                    }
                });
            return;
        }

        // Save current output device
        auto config = dm.getAudioDeviceSetup();
        savedOutputDevice = config.outputDeviceName;

        // Switch to monitor device
        config.outputDeviceName = monitorDeviceName;
        dm.setAudioDeviceSetup(config, true);

        isMonitoring = true;
    } else {
        // Restore original output device
        auto config = dm.getAudioDeviceSetup();
        config.outputDeviceName = savedOutputDevice;
        dm.setAudioDeviceSetup(config, true);

        isMonitoring = false;
    }
    repaint();
#endif
}

// --- ListBoxModel ---

int EasyEffectsAudioProcessorEditor::getNumRows() {
    return (int)activeSlots.size();
}

void EasyEffectsAudioProcessorEditor::paintListBoxItem(int, juce::Graphics&, int, int, bool) {}

juce::Component* EasyEffectsAudioProcessorEditor::refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existing) {
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
    } else if (slot.typeId == "convolver") {
        currentEditor = std::make_unique<eeval::ui::ConvolverEditor>(
            audioProcessor, slot.slotIndex);
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

    menu.addItem(1, "Save" + juce::String(pm.hasActivePreset() ? " (" + pm.getActivePresetName() + ")" : ""));
    menu.addItem(2, "Save As...");
    menu.addSeparator();

    auto list = pm.getGlobalPresetList();
    juce::PopupMenu loadMenu;
    for (int i = 0; i < list.size(); ++i)
        loadMenu.addItem(100 + i, list[i]);
    menu.addSubMenu("Load", loadMenu, list.size() > 0);

    juce::PopupMenu deleteMenu;
    for (int i = 0; i < list.size(); ++i)
        deleteMenu.addItem(200 + i, list[i]);
    menu.addSubMenu("Delete", deleteMenu, list.size() > 0);

    // Monitor device setting
    menu.addSeparator();
    menu.addItem(3, "Set Monitor Device...");

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&presetBtn),
        [this, &pm, list](int result) {
            if (result == 1) {
                if (pm.hasActivePreset()) {
                    pm.saveGlobalPreset(pm.getActivePresetName());
                } else {
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
            } else if (result == 3) {
                // Set monitor device
            #if JucePlugin_Build_Standalone
                auto* holder = juce::StandalonePluginHolder::getInstance();
                if (!holder) return;

                juce::PopupMenu deviceMenu;
                auto* deviceType = holder->deviceManager.getCurrentDeviceTypeObject();
                juce::StringArray outputDevices;
                if (deviceType)
                    outputDevices = deviceType->getDeviceNames(false);

                for (int i = 0; i < outputDevices.size(); ++i) {
                    bool isCurrent = (outputDevices[i] == monitorDeviceName);
                    deviceMenu.addItem(i + 1, outputDevices[i], true, isCurrent);
                }

                deviceMenu.showMenuAsync({}, [this, outputDevices](int r) {
                    if (r > 0) {
                        monitorDeviceName = outputDevices[r - 1];
                        if (auto* h = juce::StandalonePluginHolder::getInstance())
                            if (auto* props = h->settings.get())
                                props->setValue("monitorDevice", monitorDeviceName);
                    }
                });
            #endif
            } else if (result >= 200 && result < 300) {
                auto name = list[result - 200].toStdString();
                auto opts = juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::WarningIcon)
                    .withTitle("Delete Preset")
                    .withMessage("Delete \"" + juce::String(name) + "\"?")
                    .withButton("Delete")
                    .withButton("Cancel");
                juce::AlertWindow::showAsync(opts, [this, name](int r) {
                    if (r == 1)
                        audioProcessor.getPresetManager().deleteGlobalPreset(name);
                });
            } else if (result >= 100 && result < 200) {
                pm.loadGlobalPreset(list[result - 100].toStdString());
                refreshSidebar();
            }
        });
}

void EasyEffectsAudioProcessorEditor::showAddEffectMenu() {
    juce::PopupMenu menu;
    const auto& types = eeval::EffectRegistry::getEffectTypes();

    for (int i = 0; i < (int)types.size(); ++i)
        menu.addItem(i + 1, types[(size_t)i].displayName);

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
