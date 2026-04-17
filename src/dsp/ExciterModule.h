#pragma once
#include "EffectModule.h"

namespace eeval {

class ExciterModule : public EffectModule {
public:
    ExciterModule();
    ~ExciterModule() override = default;

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

    std::string moduleId = "exciter";
    std::string name = "Exciter";
};

} // namespace eeval
