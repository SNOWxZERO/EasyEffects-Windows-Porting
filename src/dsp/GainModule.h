#pragma once

#include "EffectModule.h"

namespace eeval {

class GainModule : public EffectModule {
public:
    GainModule();
    ~GainModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    juce::dsp::Gain<float> gainNode;
    std::string moduleId = "gain";
    std::string name = "Gain";
};

} // namespace eeval
