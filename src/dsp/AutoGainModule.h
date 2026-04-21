#pragma once

#include "EffectModule.h"

namespace eeval {

/**
 * Auto Gain Module
 * Normalizes signal level to a target LUFS (momentary approximation).
 * Uses K-weighting filters for perceptual loudness estimation.
 */
class AutoGainModule : public EffectModule {
public:
    AutoGainModule();
    ~AutoGainModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    std::string moduleId = "autogain";
    std::string name = "Auto Gain";

    struct Params {
        float target = -23.0f; // LUFS
        float attack = 100.0f;  // ms
        float release = 500.0f; // ms
        float silenceThreshold = -60.0f; // dB
        float maxGainBoost = 12.0f; // dB safety clamp
    } params;

    // K-weighting filter stages (Stereo)
    juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Filter<float>> kFilterL, kFilterR;
    
    // Envelope follower for momentary loudness
    juce::dsp::BallisticsFilter<float> envelopeFollower;
    
    // Smooth final gain
    juce::LinearSmoothedValue<float> smoothedGain{ 1.0f };
    float currentGain = 1.0f;

    void updateKFilters();
    double currentSampleRate = 44100.0;
};

} // namespace eeval
