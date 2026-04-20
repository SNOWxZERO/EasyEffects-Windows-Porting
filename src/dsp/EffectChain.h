#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include <mutex>
#include "EffectModule.h"

namespace eeval {

// Slot-based dynamic effect chain with thread-safe module swapping.
// UI thread creates/destroys modules. Audio thread only reads the active chain.
class EffectChain {
public:
    static constexpr int MAX_SLOTS = 16;

    EffectChain();
    ~EffectChain() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    void updateParameters(juce::AudioProcessorValueTreeState& apvts);

    // --- Slot management (call from UI thread only) ---

    // Set the module in a specific slot (nullptr = empty slot)
    void setSlotModule(int slotIndex, std::unique_ptr<EffectModule> module);

    // Remove the module from a slot
    void clearSlot(int slotIndex);

    // Swap two slots (for reordering)
    void swapSlots(int indexA, int indexB);

    // Get the module in a slot (for UI query, may be nullptr)
    EffectModule* getSlotModule(int slotIndex) const;

    // Get number of active (non-null) slots
    int getActiveSlotCount() const;

    // Get the type ID string stored for a slot
    const std::string& getSlotTypeId(int slotIndex) const;
    void setSlotTypeId(int slotIndex, const std::string& typeId);

    // Legacy compatibility
    void addModule(std::unique_ptr<EffectModule> module);
    void clearModules();
    std::vector<std::string> getModuleNames() const;
    std::vector<std::string> getModuleIds() const;

private:
    // Each slot holds an optional module and its type ID
    struct Slot {
        std::unique_ptr<EffectModule> module;
        std::string typeId = "none";
    };

    std::array<Slot, MAX_SLOTS> slots;

    // Cached process spec for preparing newly added modules
    juce::dsp::ProcessSpec cachedSpec{0, 0, 0};
    bool isPrepared = false;

    // Mutex for thread-safe slot modification
    // Audio thread uses try_lock, UI thread uses lock
    mutable std::mutex slotMutex;
};

} // namespace eeval
