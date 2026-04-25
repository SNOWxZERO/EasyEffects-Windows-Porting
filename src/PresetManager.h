#pragma once

#include <JuceHeader.h>

class EasyEffectsAudioProcessor;

namespace eeval {

/**
 * PresetManager - Handles serialization of the full DSP chain to JSON.
 * Supports Global presets (full chain) and Module-specific presets.
 */
class PresetManager {
public:
    PresetManager(EasyEffectsAudioProcessor& p);

    void saveGlobalPreset(const std::string& name);
    void loadGlobalPreset(const std::string& name);

    void saveModulePreset(int slotIndex, const std::string& name);
    void loadModulePreset(int slotIndex, const std::string& name);

    juce::StringArray getGlobalPresetList();
    juce::StringArray getModulePresetList(int slotIndex);
    
private:
    EasyEffectsAudioProcessor& processor;
    
    juce::File getPresetFolder(bool isGlobal);
    
    juce::var serializeChain();
    void deserializeChain(const juce::var& json);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};

} // namespace eeval
