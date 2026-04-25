#pragma once

#include <JuceHeader.h>
#include "dsp/EffectChain.h"
#include "dsp/EffectRegistry.h"
#include "dsp/SpectrumAnalyzer.h"
#include "PresetManager.h"

namespace eeval { class LevelMeterModule; }

class EasyEffectsAudioProcessor : public juce::AudioProcessor
{
public:
    EasyEffectsAudioProcessor();
    ~EasyEffectsAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    juce::AudioProcessorValueTreeState parameters;

    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    // --- Dynamic Chain Management (UI thread) ---

    // Add an effect to the next available slot, returns slot index or -1
    int addEffect(const std::string& typeId);

    // Remove an effect from a slot
    void removeEffect(int slotIndex);

    // Move an effect up or down in the chain
    void moveEffect(int slotIndex, int direction); // -1 = up, +1 = down

    // Get chain info for UI
    struct SlotInfo {
        int slotIndex;
        std::string typeId;
        std::string displayName;
        bool active;
    };
    std::vector<SlotInfo> getActiveSlots() const;

    // Editor state
    int getSelectedEditorIndex() const { return selectedEditorIndex; }
    void setSelectedEditorIndex(int index) { selectedEditorIndex = index; }

    // For LevelMeter UI polling
    eeval::LevelMeterModule* getLevelMeter();

    // Analyzer Access
    eeval::dsp::SpectrumAnalyzer& getSpectrumAnalyzer() { return spectrumAnalyzer; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Access to chain for UI queries
    eeval::EffectChain& getChain() { return dspChain; }
    eeval::PresetManager& getPresetManager() { return *presetManager; }

private:
    eeval::EffectChain dspChain;
    eeval::LevelMeterModule* levelMeterPtr = nullptr;
    std::unique_ptr<eeval::PresetManager> presetManager;
    eeval::dsp::SpectrumAnalyzer spectrumAnalyzer;
    int selectedEditorIndex = 0;

    // Rebuild the DSP chain from current slot type parameters
    void rebuildChainFromSlotTypes();

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EasyEffectsAudioProcessor)
};
