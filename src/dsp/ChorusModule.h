#pragma once
#include "EffectModule.h"

namespace eeval {

class ChorusModule : public EffectModule {
public:
    ChorusModule();
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void reset() override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    std::string moduleId = "chorus";
    std::string name = "Chorus";

    juce::dsp::Chorus<float> chorusNode;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusModule)
};

} // namespace eeval
