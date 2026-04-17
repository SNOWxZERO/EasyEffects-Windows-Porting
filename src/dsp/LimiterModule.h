#pragma once
#include "EffectModule.h"

namespace eeval {

class LimiterModule : public EffectModule {
public:
    LimiterModule();
    ~LimiterModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;
    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    juce::dsp::Limiter<float> limiterNode;
    std::string moduleId = "limiter";
    std::string name = "Limiter";
};

} // namespace eeval
