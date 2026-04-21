#include "AutoGainModule.h"
#include <cmath>

namespace eeval {

AutoGainModule::AutoGainModule() {
}

void AutoGainModule::prepare(const juce::dsp::ProcessSpec& spec) {
    currentSampleRate = spec.sampleRate;
    
    kFilterL.prepare(spec);
    kFilterR.prepare(spec);
    
    envelopeFollower.prepare(spec);
    smoothedGain.reset(spec.sampleRate, 0.1); // 100ms smoothing for gain
    
    updateKFilters();
    prepareBuffers(spec);
}

void AutoGainModule::reset() {
    kFilterL.reset();
    kFilterR.reset();
    envelopeFollower.reset();
}

void AutoGainModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    params.target = loadFloat(paramId("target"), -23.0f);
    params.attack = loadFloat(paramId("attack"), 100.0f);
    params.release = loadFloat(paramId("release"), 500.0f);
    params.silenceThreshold = loadFloat(paramId("silence_threshold"), -60.0f);

    envelopeFollower.setAttackTime(params.attack);
    envelopeFollower.setReleaseTime(params.release);

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

void AutoGainModule::updateKFilters() {
    // Stage 1: High-shelf (+4 dB at 1.5 kHz)
    auto shelf = juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, 1500.0f, 0.707f, juce::Decibels::decibelsToGain(4.0f));
    // Stage 2: High-pass (100 Hz 12dB/oct)
    auto hp = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, 100.0f, 0.707f);
    
    *kFilterL.get<0>().coefficients = *shelf;
    *kFilterL.get<1>().coefficients = *hp;
    *kFilterR.get<0>().coefficients = *shelf;
    *kFilterR.get<1>().coefficients = *hp;
}

void AutoGainModule::processInternal(juce::AudioBuffer<float>& buffer) {
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i) {
        float sumSq = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            float x = buffer.getSample(ch, i);
            
            // 1. K-Filtering (perceptual weighting)
            float kWeightX = x;
            if (ch == 0) {
                kWeightX = kFilterL.get<0>().processSample(kWeightX);
                kWeightX = kFilterL.get<1>().processSample(kWeightX);
            } else {
                kWeightX = kFilterR.get<0>().processSample(kWeightX);
                kWeightX = kFilterR.get<1>().processSample(kWeightX);
            }
            sumSq += kWeightX * kWeightX;
        }
        
        // Mean power
        float meanPower = sumSq / (float)numChannels;
        
        // 2. Momentary Loudness Estimation via Ballistics
        // Ballistics filter expects level, not power, so we use sqrt(meanPower) = RMS
        float rms = std::sqrt(meanPower);
        float env = envelopeFollower.processSample(0, rms);
        
        // 3. Gain Calculation
        float loudnessDb = juce::Decibels::gainToDecibels(env, -100.0f) - 0.691f; // BS.1770 offset
        
        if (loudnessDb > params.silenceThreshold) {
            float gainRequiredDb = params.target - loudnessDb;
            
            // Safety clamp
            gainRequiredDb = std::clamp(gainRequiredDb, -params.maxGainBoost, params.maxGainBoost);
            
            float targetGainLinear = juce::Decibels::decibelsToGain(gainRequiredDb);
            smoothedGain.setTargetValue(targetGainLinear);
        }
        
        float finalGain = smoothedGain.getNextValue();
        
        // 4. Apply Gain
        for (int ch = 0; ch < numChannels; ++ch) {
            buffer.setSample(ch, i, buffer.getSample(ch, i) * finalGain);
        }
    }
}

const std::string& AutoGainModule::getModuleId() const {
    return moduleId;
}

const std::string& AutoGainModule::getName() const {
    return name;
}

} // namespace eeval
