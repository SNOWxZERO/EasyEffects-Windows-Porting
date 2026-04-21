#include "ExpanderModule.h"
#include <cmath>

namespace eeval {

ExpanderModule::ExpanderModule() {
}

void ExpanderModule::prepare(const juce::dsp::ProcessSpec& spec) {
    currentSampleRate = spec.sampleRate;
    
    envelopeFollower.prepare(spec);
    smoothedGain.reset(spec.sampleRate, 0.02); // 20ms smoothing for gain changes
    
    prepareBuffers(spec);
}

void ExpanderModule::reset() {
    envelopeFollower.reset();
    smoothedGain.reset(currentSampleRate, 0.02);
}

void ExpanderModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    params.threshold = loadFloat(paramId("threshold"), -40.0f);
    params.ratio = loadFloat(paramId("ratio"), 2.0f);
    params.attack = loadFloat(paramId("attack"), 10.0f);
    params.release = loadFloat(paramId("release"), 100.0f);
    params.knee = loadFloat(paramId("knee"), 0.0f);
    
    float gainDb = loadFloat(paramId("makeup_gain"), 0.0f);
    params.makeupGain = juce::Decibels::decibelsToGain(gainDb);

    envelopeFollower.setAttackTime(params.attack);
    envelopeFollower.setReleaseTime(params.release);

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

float ExpanderModule::calculateGain(float envelope) {
    float envDb = juce::Decibels::gainToDecibels(envelope, -100.0f);
    float grDb = 0.0f;

    // Soft knee implementation
    if (params.knee > 0.0f) {
        float kneeStart = params.threshold - params.knee / 2.0f;
        float kneeEnd = params.threshold + params.knee / 2.0f;

        if (envDb < kneeStart) {
            // Full expansion
            grDb = (params.threshold - envDb) * (params.ratio - 1.0f);
        } else if (envDb < kneeEnd) {
            // Transition zone (cubic or quadratic)
            float t = (kneeEnd - envDb) / params.knee; // 0 at end, 1 at start
            float kneeGr = (params.threshold - envDb) * (params.ratio - 1.0f);
            grDb = kneeGr * (t * t); // Simple quadratic blend
        }
    } else {
        if (envDb < params.threshold) {
            grDb = (params.threshold - envDb) * (params.ratio - 1.0f);
        }
    }

    return juce::Decibels::decibelsToGain(-grDb);
}

void ExpanderModule::processInternal(juce::AudioBuffer<float>& buffer) {
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i) {
        // Find peak of all channels for envelope detection
        float peak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            peak = std::max(peak, std::abs(buffer.getSample(ch, i)));
        }

        float envelope = envelopeFollower.processSample(0, peak);
        float targetGain = calculateGain(envelope);
        smoothedGain.setTargetValue(targetGain);

        float currentGain = smoothedGain.getNextValue() * params.makeupGain;

        for (int ch = 0; ch < numChannels; ++ch) {
            buffer.setSample(ch, i, buffer.getSample(ch, i) * currentGain);
        }
    }
}

const std::string& ExpanderModule::getModuleId() const {
    return moduleId;
}

const std::string& ExpanderModule::getName() const {
    return name;
}

} // namespace eeval
