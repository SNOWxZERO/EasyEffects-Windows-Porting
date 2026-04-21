#pragma once

#include "EffectModule.h"

namespace eeval {

/**
 * Bass Loudness Module
 * Inspired by the MDA Loudness algorithm.
 * Provides equal-loudness compensation (bass boost) that is level-dependent (optional) or fixed.
 */
class BassLoudnessModule : public EffectModule {
public:
    BassLoudnessModule();
    ~BassLoudnessModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    std::string moduleId = "bass_loudness";
    std::string name = "Bass Loudness";

    struct Params {
        float loudness = 0.0f; // dB
        float output = 0.0f;   // dB
        bool link = false;
    } params;

    // We implement the loudness curve using a high-quality low-shelf filter
    juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>> shelfL, shelfR;
    
    // Smooth gain for the loudness compensation
    juce::LinearSmoothedValue<float> smoothedLoudnessGain{ 1.0f };
    juce::LinearSmoothedValue<float> smoothedOutputGain{ 1.0f };

    void updateFilters();
    double currentSampleRate = 44100.0;
};

} // namespace eeval
