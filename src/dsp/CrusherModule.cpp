#include "CrusherModule.h"
#include <cmath>
#include <algorithm>

namespace eeval {

CrusherModule::CrusherModule() {
}

void CrusherModule::prepare(const juce::dsp::ProcessSpec& spec) {
    currentSampleRate = spec.sampleRate;
    
    smoothedBits.reset(spec.sampleRate, 0.05);
    smoothedSamples.reset(spec.sampleRate, 0.05);
    
    prepareBuffers(spec);
}

void CrusherModule::reset() {
    lastSampleL = 0.0f;
    lastSampleR = 0.0f;
    phase = 0.0f;
}

void CrusherModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    params.bitDepth = apvts.getRawParameterValue(paramId("bits"))->load();
    params.sampleRateReduction = apvts.getRawParameterValue(paramId("samples"))->load();
    params.jitter = apvts.getRawParameterValue(paramId("jitter"))->load();

    smoothedBits.setTargetValue(params.bitDepth);
    smoothedSamples.setTargetValue(params.sampleRateReduction);

    setDryWetMix(apvts.getRawParameterValue(paramId("mix"))->load() / 100.0f);
    setBypassed(apvts.getRawParameterValue(paramId("bypass"))->load() > 0.5f);
}

void CrusherModule::processInternal(juce::AudioBuffer<float>& buffer) {
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();
    
    auto* left = buffer.getWritePointer(0);
    auto* right = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i) {
        float currentBits = smoothedBits.getNextValue();
        float currentSRR = smoothedSamples.getNextValue();
        
        // 1. Double-buffered sample-and-hold (Downsampling)
        float invSrr = 1.0f / currentSRR;
        phase += invSrr;
        
        if (phase >= 1.0f) {
            phase -= 1.0f;
            lastSampleL = left[i];
            if (right) lastSampleR = right[i];
            
            // Apply bit reduction only when taking a new sample
            float levels = std::pow(2.0f, currentBits);
            
            auto bitCrush = [&](float in) {
                if (currentBits >= 16.0f) return in; // No crushing at "16 bits" for performance
                float s = std::clamp(in, -1.0f, 1.0f);
                return std::round(s * levels) / levels;
            };

            lastSampleL = bitCrush(lastSampleL);
            if (right) lastSampleR = bitCrush(lastSampleR);
        }

        left[i] = lastSampleL;
        if (right) right[i] = lastSampleR;
    }
}

const std::string& CrusherModule::getModuleId() const {
    return moduleId;
}

const std::string& CrusherModule::getName() const {
    return name;
}

} // namespace eeval
