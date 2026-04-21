#pragma once

#include "EffectModule.h"

namespace eeval {

/**
 * Crystalizer Module
 * A multiband peak enhancer based on the second derivative of the signal.
 * This implementation uses a simplified 3-band crossover system.
 */
class CrystalizerModule : public EffectModule {
public:
    CrystalizerModule();
    ~CrystalizerModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    std::string moduleId = "crystalizer";
    std::string name = "Crystalizer";

    struct Params {
        float intensityLow = 0.0f;
        float intensityMid = 0.0f;
        float intensityHigh = 0.0f;
        bool adaptiveIntensity = true;
    } params;

    // Crossover frequencies
    const float lowMidFreq = 200.0f;
    const float midHighFreq = 3000.0f;

    // Multiband splitter using Linkwitz-Riley 4th order (2 crossovers)
    using CrossOver = juce::dsp::LinkwitzRileyFilter<float>;
    CrossOver cross1L, cross1R; // Split Low / (Mid+High)
    CrossOver cross2L, cross2R; // Split Mid / High

    // State for second derivative calculation (x[n-1]) for each channel and band
    // Band 0: Low, 1: Mid, 2: High
    std::array<std::array<float, 3>, 2> lastSamples; // [channel][band]
    std::array<std::array<float, 3>, 2> lastProcessedSamples; // [channel][band]

    float processBand(int channel, int band, float input, float intensity);

    double currentSampleRate = 44100.0;
};

} // namespace eeval
