#pragma once
#include "EffectModule.h"

namespace eeval {

class DeesserModule : public EffectModule {
public:
    DeesserModule();
    ~DeesserModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;
    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    juce::dsp::LinkwitzRileyFilter<float> lpFilter;
    juce::dsp::LinkwitzRileyFilter<float> hpFilter;
    
    juce::dsp::Compressor<float> compressorNode;
    
    juce::AudioBuffer<float> lowBandBuffer;
    juce::AudioBuffer<float> highBandBuffer;

    std::string moduleId = "deesser";
    std::string name = "Deesser";
};

} // namespace eeval
