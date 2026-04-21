#pragma once

#include "EffectModule.h"

namespace eeval {

/**
 * Crusher Module (Bitcrusher / Downsampler)
 * Digital distortion through quantization and sample-rate reduction.
 */
class CrusherModule : public EffectModule {
public:
    CrusherModule();
    ~CrusherModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    std::string moduleId = "crusher";
    std::string name = "Crusher";

    struct Params {
        float bitDepth = 16.0f;
        float sampleRateReduction = 1.0f;
        float jitter = 0.0f;
    } params;

    juce::LinearSmoothedValue<float> smoothedBits{ 16.0f };
    juce::LinearSmoothedValue<float> smoothedSamples{ 1.0f };

    float lastSampleL = 0.0f;
    float lastSampleR = 0.0f;
    float phase = 0.0f;

    double currentSampleRate = 44100.0;
};

} // namespace eeval
