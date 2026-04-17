#pragma once
#include "EffectModule.h"

namespace eeval {

class ConvolverModule : public EffectModule {
public:
    ConvolverModule();
    ~ConvolverModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;
    const std::string& getModuleId() const override;
    const std::string& getName() const override;

    // Call this to load an Impulse Response File
    void loadImpulseResponse(const juce::File& file);

private:
    juce::dsp::Convolution convolverNode;
    bool hasIRLoaded = false;

    std::string moduleId = "convolver";
    std::string name = "Convolver";
};

} // namespace eeval
