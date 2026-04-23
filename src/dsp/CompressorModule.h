#pragma once

#include "EffectModule.h"

namespace eeval {

class CompressorModule : public EffectModule {
public:
    CompressorModule();
    ~CompressorModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;
    float getGainReduction() const override { return gr.load(); }

private:
    juce::dsp::Compressor<float> compressorNode;
    std::string moduleId = "compressor";
    std::string name = "Compressor";
    std::atomic<float> gr { 0.0f };
};

} // namespace eeval
