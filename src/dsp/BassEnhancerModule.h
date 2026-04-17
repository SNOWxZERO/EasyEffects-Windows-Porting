#pragma once
#include "EffectModule.h"

namespace eeval {

class BassEnhancerModule : public EffectModule {
public:
    BassEnhancerModule();
    ~BassEnhancerModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;
    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    juce::dsp::LinkwitzRileyFilter<float> lpFilter;
    juce::dsp::LinkwitzRileyFilter<float> hpFilter;
    
    juce::dsp::WaveShaper<float, std::function<float(float)>> waveShaper;
    
    juce::AudioBuffer<float> lowBandBuffer;
    juce::AudioBuffer<float> highBandBuffer;
    
    float amount = 0.0f;
    float harmonics = 0.5f;

    std::string moduleId = "bassenhancer";
    std::string name = "Bass Enhancer";
};

} // namespace eeval
