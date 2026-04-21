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
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    params.balanceIn = loadFloat(paramId("balance_in"), 0.0f);
    params.balanceOut = loadFloat(paramId("balance_out"), 0.0f);
    params.mode = static_cast<Mode>(static_cast<int>(loadFloat(paramId("mode"), 0.0f)));
    
    float sLevelDb = loadFloat(paramId("side_level"), 0.0f);
    params.sideLevel = juce::Decibels::decibelsToGain(sLevelDb);
    smoothedSideLevel.setTargetValue(params.sideLevel);

    params.sideBalance = loadFloat(paramId("side_balance"), 0.0f);

    float mLevelDb = loadFloat(paramId("middle_level"), 0.0f);
    params.middleLevel = juce::Decibels::decibelsToGain(mLevelDb);
    smoothedMidLevel.setTargetValue(params.middleLevel);

    params.middlePanorama = loadFloat(paramId("middle_panorama"), 0.0f);
    
    float sb = loadFloat(paramId("stereo_base"), 0.0f);
    // stereoBase -1 to 1 maps to width 0 to 2
    params.stereoBase = sb + 1.0f;
    smoothedWidth.setTargetValue(params.stereoBase);

    params.delayMs = loadFloat(paramId("delay"), 0.0f);
    params.stereoPhase = loadFloat(paramId("stereo_phase"), 0.0f);
    
    params.muteL = loadFloat(paramId("mute_l"), 0.0f) > 0.5f;
    params.muteR = loadFloat(paramId("mute_r"), 0.0f) > 0.5f;
    params.invertL = loadFloat(paramId("invert_l"), 0.0f) > 0.5f;
    params.invertR = loadFloat(paramId("invert_r"), 0.0f) > 0.5f;

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
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
