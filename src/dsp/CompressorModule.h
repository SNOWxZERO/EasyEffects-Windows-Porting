#pragma once

#include "EffectModule.h"

namespace eeval {

class CompressorModule : public EffectModule {
public:
    CompressorModule();
    ~CompressorModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getName() const override;

private:
    juce::dsp::Compressor<float> compressorNode;
    std::string name = "Compressor";
};

} // namespace eeval
