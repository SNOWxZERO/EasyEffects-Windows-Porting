#pragma once
#include "EffectModule.h"

namespace eeval {

class ReverbModule : public EffectModule {
public:
    ReverbModule();
    ~ReverbModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;
    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    juce::dsp::Reverb reverbNode;
    juce::dsp::Reverb::Parameters reverbParams;
    std::string moduleId = "reverb";
    std::string name = "Reverb";
};

} // namespace eeval
