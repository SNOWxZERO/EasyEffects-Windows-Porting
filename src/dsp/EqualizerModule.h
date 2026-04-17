#pragma once

#include "EffectModule.h"
#include <vector>
#include <array>

namespace eeval {

class EqualizerModule : public EffectModule {
public:
    // Number of bands — change this single value to scale the EQ
    static constexpr int NUM_BANDS = 4;

    EqualizerModule();
    ~EqualizerModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    // One filter per band per channel (stereo = 2 channels)
    // filters[band][channel]
    std::vector<std::array<juce::dsp::IIR::Filter<float>, 2>> filters;

    double currentSampleRate = 44100.0;
    std::string moduleId = "eq";
    std::string name = "Equalizer";

    void updateCoefficients(int bandIndex, float gain, float freq, float q);
};

} // namespace eeval
