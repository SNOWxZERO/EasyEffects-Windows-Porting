#pragma once

#include "EffectModule.h"

namespace eeval {

/**
 * Crossfeed Module (Bauer BS2B approximation)
 * Simulates speaker-like imaging by frequency-dependent crosstalk.
 */
class CrossfeedModule : public EffectModule {
public:
    CrossfeedModule();
    ~CrossfeedModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    std::string moduleId = "crossfeed";
    std::string name = "Crossfeed";

    struct Params {
        float fcut = 700.0f;
        float feed = 4.5f; // in dB
    } params;

    // Filters for L and R channels
    // Each channel has a direct high-shelf and a cross low-pass
    juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>> directChainL, directChainR;
    juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>> crossChainL, crossChainR;

    void updateFilters();
    double currentSampleRate = 44100.0;
};

} // namespace eeval
