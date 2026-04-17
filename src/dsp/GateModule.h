#pragma once

#include "EffectModule.h"

namespace eeval {

class GateModule : public EffectModule {
public:
    GateModule();
    ~GateModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    juce::dsp::NoiseGate<float> gateNode;
    std::string moduleId = "gate";
    std::string name = "Gate";
};

} // namespace eeval
