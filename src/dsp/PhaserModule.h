#pragma once
#include "EffectModule.h"

namespace eeval {

class PhaserModule : public EffectModule {
public:
    PhaserModule();
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void reset() override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    std::string moduleId = "phaser";
    std::string name = "Phaser";

    juce::dsp::Phaser<float> phaserNode;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaserModule)
};

} // namespace eeval
