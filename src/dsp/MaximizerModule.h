#pragma once

#include "EffectModule.h"

namespace eeval {

/**
 * Maximizer Module
 * Look-ahead brickwall limiter designed for final gain boosting.
 */
class MaximizerModule : public EffectModule {
public:
    MaximizerModule();
    ~MaximizerModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;
    double getLatencySamples() const override;

private:
    std::string moduleId = "maximizer";
    std::string name = "Maximizer";

    struct Params {
        float threshold = 0.0f;
        float ceiling = 0.0f;
        float release = 10.0f;
        bool softClip = false;
    } params;

    juce::dsp::Limiter<float> limiter;
    double currentSampleRate = 44100.0;
    
    // Soft clipper
    float applySoftClip(float in);
};

} // namespace eeval
