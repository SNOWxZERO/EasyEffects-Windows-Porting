#include "StereoToolsModule.h"
#include <cmath>

namespace eeval {

StereoToolsModule::StereoToolsModule() : delayLine(960) { // ~20ms at 48k is 960 samples
}

void StereoToolsModule::prepare(const juce::dsp::ProcessSpec& spec) {
    currentSampleRate = spec.sampleRate;
    delayLine.prepare(spec);
    
    smoothedSideLevel.reset(spec.sampleRate, 0.05);
    smoothedMidLevel.reset(spec.sampleRate, 0.05);
    smoothedWidth.reset(spec.sampleRate, 0.05);
    
    prepareBuffers(spec);
}

void StereoToolsModule::reset() {
    delayLine.reset();
}

void StereoToolsModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    params.balanceIn = apvts.getRawParameterValue(paramId("balance_in"))->load();
    params.balanceOut = apvts.getRawParameterValue(paramId("balance_out"))->load();
    params.mode = static_cast<Mode>(static_cast<int>(apvts.getRawParameterValue(paramId("mode"))->load()));
    
    float sLevelDb = apvts.getRawParameterValue(paramId("side_level"))->load();
    params.sideLevel = juce::Decibels::decibelsToGain(sLevelDb);
    smoothedSideLevel.setTargetValue(params.sideLevel);

    params.sideBalance = apvts.getRawParameterValue(paramId("side_balance"))->load();

    float mLevelDb = apvts.getRawParameterValue(paramId("middle_level"))->load();
    params.middleLevel = juce::Decibels::decibelsToGain(mLevelDb);
    smoothedMidLevel.setTargetValue(params.middleLevel);

    params.middlePanorama = apvts.getRawParameterValue(paramId("middle_panorama"))->load();
    
    float sb = apvts.getRawParameterValue(paramId("stereo_base"))->load();
    // stereoBase -1 to 1 maps to width 0 to 2
    params.stereoBase = sb + 1.0f;
    smoothedWidth.setTargetValue(params.stereoBase);

    params.delayMs = apvts.getRawParameterValue(paramId("delay"))->load();
    params.stereoPhase = apvts.getRawParameterValue(paramId("stereo_phase"))->load();
    
    params.muteL = apvts.getRawParameterValue(paramId("mute_l"))->load() > 0.5f;
    params.muteR = apvts.getRawParameterValue(paramId("mute_r"))->load() > 0.5f;
    params.invertL = apvts.getRawParameterValue(paramId("invert_l"))->load() > 0.5f;
    params.invertR = apvts.getRawParameterValue(paramId("invert_r"))->load() > 0.5f;

    setDryWetMix(apvts.getRawParameterValue(paramId("mix"))->load() / 100.0f);
    setBypassed(apvts.getRawParameterValue(paramId("bypass"))->load() > 0.5f);
}

void StereoToolsModule::processInternal(juce::AudioBuffer<float>& buffer) {
    auto numSamples = buffer.getNumSamples();
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    if (!right) return; // Stereo tools needs stereo

    for (int i = 0; i < numSamples; ++i) {
        float l = left[i];
        float r = right[i];

        // 1. Balance In
        if (params.balanceIn < 0.0f) {
            r *= (1.0f + params.balanceIn);
        } else if (params.balanceIn > 0.0f) {
            l *= (1.0f - params.balanceIn);
        }

        // 2. Routing Mode & Mid/Side Processing
        float mid = (l + r) * 0.5f;
        float side = (l - r) * 0.5f;

        // Apply Mid/Side modifiers
        mid *= smoothedMidLevel.getNextValue();
        side *= smoothedSideLevel.getNextValue();

        // Mid Panorama
        if (params.middlePanorama < 0.0f) {
            // Pan Mid left? In Calf this usually means moving the mid signal
            // For now simple panners on M/S
        }
        
        // Width adjustment
        float width = smoothedWidth.getNextValue();
        float outL = mid + width * side;
        float outR = mid - width * side;

        // 3. Mode selection
        switch (params.mode) {
            case Mode::LR_LR: 
                break;
            case Mode::LR_MS:
                outL = mid;
                outR = side;
                break;
            case Mode::MS_LR:
                // Treat input as M/S and decode
                mid = l;
                side = r;
                outL = mid + side;
                outR = mid - side;
                break;
            case Mode::LR_LL:
                outL = l;
                outR = l;
                break;
            case Mode::LR_RR:
                outL = r;
                outR = r;
                break;
            case Mode::LR_LplusR:
                outL = l + r;
                outR = l + r;
                break;
            case Mode::LR_RL:
                outL = r;
                outR = l;
                break;
        }

        // 4. Delay (Haas Effect)
        if (params.delayMs != 0.0f) {
            float delaySamples = std::abs(params.delayMs) * (float)currentSampleRate / 1000.0f;
            if (params.delayMs > 0.0f) {
                // Delay Right
                delayLine.pushSample(1, outR);
                outR = delayLine.popSample(1, delaySamples);
            } else {
                // Delay Left
                delayLine.pushSample(0, outL);
                outL = delayLine.popSample(0, delaySamples);
            }
        }

        // 5. Phase Inversion & Mute
        if (params.invertL) outL = -outL;
        if (params.invertR) outR = -outR;
        if (params.muteL) outL = 0.0f;
        if (params.muteR) outR = 0.0f;

        // 6. Balance Out
        if (params.balanceOut < 0.0f) {
            outR *= (1.0f + params.balanceOut);
        } else if (params.balanceOut > 0.0f) {
            outL *= (1.0f - params.balanceOut);
        }

        left[i] = outL;
        right[i] = outR;
    }
}

const std::string& StereoToolsModule::getModuleId() const {
    return moduleId;
}

const std::string& StereoToolsModule::getName() const {
    return name;
}

} // namespace eeval
