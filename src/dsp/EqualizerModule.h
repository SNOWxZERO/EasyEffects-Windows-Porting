#pragma once

#include "EffectModule.h"
#include <vector>
#include <array>

namespace eeval {

/**
 * Enhanced Parametric Equalizer Module.
 * Supports up to 10 bands with selectable filter types.
 */
class EqualizerModule : public EffectModule {
public:
    static constexpr int NUM_BANDS = 10;

    enum class FilterType {
        Peak = 0,
        LowPass,
        HighPass,
        LowShelf,
        HighShelf,
        Notch
    };

    EqualizerModule();
    ~EqualizerModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

    // Helper for UI to query magnitude response (Thread-safe)
    float getMagnitudeForFrequency(double frequency) const;

private:
    struct BandState {
        bool enabled = true;
        FilterType type = FilterType::Peak;
        float freq = 1000.0f;
        float gain = 0.0f;
        float q = 0.707f;
        
        // Internal filters for processing
        std::array<juce::dsp::IIR::Filter<float>, 2> filters;
    };

    std::vector<BandState> bands;

    double currentSampleRate = 44100.0;
    std::string moduleId = "eq";
    std::string name = "Equalizer";

    void updateBandCoefficients(int bandIndex);
};

} // namespace eeval
