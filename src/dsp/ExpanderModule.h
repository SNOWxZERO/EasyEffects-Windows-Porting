#pragma once

#include "EffectModule.h"

namespace eeval {

/**
 * Downward Expander Module
 * Reduces gain when signal is below threshold.
 */
class ExpanderModule : public EffectModule {
public:
    ExpanderModule();
    ~ExpanderModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    std::string moduleId = "expander";
    std::string name = "Expander";

    struct Params {
        float threshold = -40.0f;
        float ratio = 2.0f;
        float attack = 10.0f;
        float release = 100.0f;
        float knee = 0.0f;
        float makeupGain = 1.0f;
    } params;

    juce::dsp::BallisticsFilter<float> envelopeFollower;
    double currentSampleRate = 44100.0;
    
    juce::LinearSmoothedValue<float> smoothedGain{ 1.0f };

    float calculateGain(float envelope);
};

} // namespace eeval
