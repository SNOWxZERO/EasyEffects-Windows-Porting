#pragma once

#include <JuceHeader.h>

class EasyEffectsAudioProcessor;

namespace eeval {

/**
 * PresetManager - Handles serialization of the full DSP chain to JSON.
 * Supports Global presets (full chain) and Module-specific presets.
 * Tracks the active preset and dirty state for save/save-as workflow.
 */
class PresetManager {
public:
    PresetManager(EasyEffectsAudioProcessor& p);

    // --- Global Presets ---
    void saveGlobalPreset(const std::string& name);   // Save (or overwrite) by name
    void loadGlobalPreset(const std::string& name);
    void deleteGlobalPreset(const std::string& name);
    juce::StringArray getGlobalPresetList();

    // --- Active Preset Tracking ---
    bool hasActivePreset() const { return !currentPresetName.empty(); }
    const std::string& getActivePresetName() const { return currentPresetName; }
    void clearActivePreset() { currentPresetName.clear(); dirty = false; }
    
    bool isDirty() const { return dirty; }
    void markDirty() { dirty = true; }

    // --- Module Presets ---
    void saveModulePreset(int slotIndex, const std::string& name);
    void loadModulePreset(int slotIndex, const std::string& name);
    void deleteModulePreset(int slotIndex, const std::string& name);
    juce::StringArray getModulePresetList(int slotIndex);
    
private:
    EasyEffectsAudioProcessor& processor;
    std::string currentPresetName;
    bool dirty = false;
    
    juce::File getPresetFolder(bool isGlobal);
    
    juce::var serializeChain();
    void deserializeChain(const juce::var& json);
    void createDefaultPresetsIfNeeded();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};

} // namespace eeval
