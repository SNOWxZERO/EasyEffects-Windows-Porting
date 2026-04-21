#pragma once

#include "EffectModule.h"

namespace eeval {

class StereoToolsModule : public EffectModule {
public:
    StereoToolsModule();
    ~StereoToolsModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

    enum class Mode {
        LR_LR = 0,
        LR_MS,
        MS_LR,
        LR_LL,
        LR_RR,
        LR_LplusR,
        LR_RL
    };

private:
    std::string moduleId = "stereotools";
    std::string name = "Stereo Tools";

    struct Params {
        float balanceIn = 0.0f;
        float balanceOut = 0.0f;
        Mode mode = Mode::LR_LR;
        float sideLevel = 0.0f;
        float sideBalance = 0.0f;
        float middleLevel = 0.0f;
        float middlePanorama = 0.0f;
        float stereoBase = 0.0f;
        float delayMs = 0.0f;
        float stereoPhase = 0.0f;
        bool muteL = false;
        bool muteR = false;
        bool invertL = false;
        bool invertR = false;
    } params;

    // Smoothed values
    juce::LinearSmoothedValue<float> smoothedSideLevel{ 0.0f };
    juce::LinearSmoothedValue<float> smoothedMidLevel{ 0.0f };
    juce::LinearSmoothedValue<float> smoothedWidth{ 1.0f };
    
    // Delay line for Haas effect
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
    double currentSampleRate = 44100.0;

    void applyStereoset(float& left, float& right);
};

} // namespace eeval
