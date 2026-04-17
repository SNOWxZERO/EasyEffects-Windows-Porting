#pragma once
#include "EffectModule.h"
#include <atomic>

namespace eeval {

class LevelMeterModule : public EffectModule {
public:
    LevelMeterModule();
    ~LevelMeterModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;
    const std::string& getModuleId() const override;
    const std::string& getName() const override;

    // UI access methods. Return smoothed values in decibels.
    float getPeak() const { return currentPeak.load(); }
    float getRms() const { return currentRms.load(); }

private:
    std::atomic<float> currentRms { -100.0f };
    std::atomic<float> currentPeak { -100.0f };
    
    // Smoothing coefficients
    float rmsDecay = 0.0f;
    float peakDecay = 0.0f;

    std::string moduleId = "meter";
    std::string name = "Level Meter";
};

} // namespace eeval
