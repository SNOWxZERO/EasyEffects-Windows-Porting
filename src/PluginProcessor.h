#pragma once

#include <JuceHeader.h>
#include "dsp/EffectChain.h"

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

    // Phase 5: Editor State & UI Synchronization
    int getSelectedEditorIndex() const { return selectedEditorIndex; }
    void setSelectedEditorIndex(int index) { selectedEditorIndex = index; }
    std::vector<std::string> getActiveEffectNames() const { 
        auto names = dspChain.getModuleNames();
        names.erase(std::remove(names.begin(), names.end(), "Level Meter"), names.end());
        return names;
    }
    std::vector<std::string> getActiveEffectIds() const {
        auto ids = dspChain.getModuleIds();
        ids.erase(std::remove(ids.begin(), ids.end(), "meter"), ids.end());
        return ids;
    }
    
    // For LevelMeter UI polling
    eeval::LevelMeterModule* getLevelMeter();

    // FFT Visualization Data (Pass to UI safely)
    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder; // 2048
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData; // 2x size for complex results
    int fifoIndex = 0;
    std::atomic<bool> nextFFTBlockReady { false };
    
    void pushNextSampleIntoFifo(float sample) noexcept {
        if (fifoIndex == fftSize) {
            if (!nextFFTBlockReady) {
                std::copy(fifo.begin(), fifo.end(), fftData.begin());
                nextFFTBlockReady = true;
            }
            fifoIndex = 0;
        }
        fifo[(size_t)fifoIndex++] = sample;
    }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

private:
    eeval::EffectChain dspChain;
    eeval::LevelMeterModule* levelMeterPtr = nullptr;
    int selectedEditorIndex = 0; // Centralized UI state
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EasyEffectsAudioProcessor)
};
