#pragma once
#include "EffectModule.h"

namespace eeval {

class FilterModule : public EffectModule {
public:
    FilterModule();
    ~FilterModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;
    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                    juce::dsp::IIR::Coefficients<float>> filterNode;
    double currentSampleRate = 44100.0;
    int currentType = 0; // 0 = highpass, 1 = lowpass
    std::string moduleId = "filter";
    std::string name = "Filter";
};

} // namespace eeval
