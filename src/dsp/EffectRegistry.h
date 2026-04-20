#pragma once
#include <JuceHeader.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "EffectModule.h"

namespace eeval {

// Describes a single parameter for an effect type
struct SlotParamDescriptor {
    std::string suffix;     // e.g., "threshold" → becomes "slotX.gate.threshold"
    std::string label;      // UI display: "Threshold"
    std::string unit;       // "dB", "ms", "%", ":1", "Hz"
    float minVal;
    float maxVal;
    float step;
    float defaultVal;
};

// Describes a choice parameter (e.g., filter type)
struct SlotChoiceDescriptor {
    std::string suffix;     // e.g., "type" → becomes "slotX.filter.type"
    std::string label;
    juce::StringArray choices;
    int defaultIndex;
};

// Full descriptor for an effect type
struct EffectTypeDescriptor {
    std::string typeId;         // e.g., "gate", "compressor", "eq"
    std::string displayName;    // e.g., "Gate", "Compressor", "Equalizer"
    std::vector<SlotParamDescriptor> params;
    std::vector<SlotChoiceDescriptor> choices;
    // Factory: creates a DSP module instance that reads from slot-prefixed parameters
    std::function<std::unique_ptr<EffectModule>(const std::string& slotPrefix)> factory;
};

// Central registry — drives DSP creation, UI generation, and preset reconstruction
class EffectRegistry {
public:
    static const int MAX_SLOTS = 16;

    // Returns all available effect types
    static const std::vector<EffectTypeDescriptor>& getEffectTypes();

    // Find a descriptor by type ID
    static const EffectTypeDescriptor* findType(const std::string& typeId);

    // Get list of type IDs for the slot choice parameter
    static juce::StringArray getTypeChoices();

    // Register all parameters for a single slot into a parameter layout
    static void registerSlotParameters(
        juce::AudioProcessorValueTreeState::ParameterLayout& layout,
        int slotIndex);

    // Create a DSP module for a given slot and type
    static std::unique_ptr<EffectModule> createModule(
        const std::string& typeId,
        const std::string& slotPrefix);

    // Build the slot prefix string: "slot0", "slot1", etc.
    static std::string slotPrefix(int slotIndex) {
        return "slot" + std::to_string(slotIndex);
    }

    // Build a full parameter ID: "slot0.gate.threshold"
    static std::string paramId(int slotIndex, const std::string& typeId, const std::string& suffix) {
        return slotPrefix(slotIndex) + "." + typeId + "." + suffix;
    }

private:
    static std::vector<EffectTypeDescriptor> buildRegistry();
};

} // namespace eeval
