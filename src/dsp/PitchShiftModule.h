#pragma once

#include "EffectModule.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <SoundTouch.h>
#include <memory>
#include <vector>

namespace eeval {

/**
 * PitchShiftModule - Time-domain pitch shifting using the SoundTouch library.
 */
class PitchShiftModule : public EffectModule {
public:
    PitchShiftModule();
    ~PitchShiftModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    double getLatencySamples() const override { return latencySamples; }

    const std::string& getModuleId() const override { return moduleId; }
    const std::string& getName() const override { return name; }

private:
    std::string moduleId = "pitch_shift";
    std::string name = "Pitch Shift";

    std::unique_ptr<soundtouch::SoundTouch> st;
    
    // Buffers for interleaved processing
    std::vector<float> interleavedBuffer;
    std::vector<float> outputInterleaved;

    std::atomic<float>* pitchParam = nullptr;
    float lastPitch = 0.0f;
    double latencySamples = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchShiftModule)
};

} // namespace eeval
