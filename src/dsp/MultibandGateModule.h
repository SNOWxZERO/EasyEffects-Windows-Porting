#pragma once
#include "EffectModule.h"
#include "BandSplitter.h"

namespace eeval {

/**
 * 3-band Noise Gate.
 * Applies independent gating to Low, Mid, and High frequency bands.
 */
class MultibandGateModule : public EffectModule {
public:
    MultibandGateModule();
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void reset() override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    std::string moduleId = "multiband_gate";
    std::string name = "Multiband Gate";

    BandSplitter splitter;
    juce::AudioBuffer<float> bandBuffers[3];

    struct GateBand {
        juce::dsp::NoiseGate<float> gate;
        void prepare(const juce::dsp::ProcessSpec& spec) { gate.prepare(spec); }
        void reset() { gate.reset(); }
    };

    GateBand bands[3];
    float lowMidFreq = 200.0f;
    float midHighFreq = 2500.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultibandGateModule)
};

} // namespace eeval
