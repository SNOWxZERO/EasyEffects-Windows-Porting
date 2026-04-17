#pragma once
#include "EffectModule.h"

namespace eeval {

class DelayModule : public EffectModule {
public:
    DelayModule();
    ~DelayModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;
    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    static constexpr float MAX_DELAY_MS = 2000.0f;
    juce::dsp::DelayLine<float> delayLine { 96000 }; // Pre-allocate max samples (2s @ 48kHz)
    float delayTimeMs = 0.0f;
    float feedback = 0.0f;
    float delayMix = 0.5f;  // Internal delay wet/dry (separate from module mix)
    double currentSampleRate = 44100.0;
    std::string moduleId = "delay";
    std::string name = "Delay";
};

} // namespace eeval
