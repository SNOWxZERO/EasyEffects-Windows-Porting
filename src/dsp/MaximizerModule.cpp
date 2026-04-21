#include "MaximizerModule.h"
#include <cmath>

namespace eeval {

MaximizerModule::MaximizerModule() {
}

void MaximizerModule::prepare(const juce::dsp::ProcessSpec& spec) {
    limiter.prepare(spec);
    prepareBuffers(spec);
}

void MaximizerModule::reset() {
    limiter.reset();
}

void MaximizerModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    params.threshold = apvts.getRawParameterValue(paramId("threshold"))->load();
    params.ceiling = apvts.getRawParameterValue(paramId("ceiling"))->load();
    params.release = apvts.getRawParameterValue(paramId("release"))->load();
    params.softClip = apvts.getRawParameterValue(paramId("soft_clip"))->load() > 0.5f;

    limiter.setThreshold(params.threshold);
    limiter.setRelease(params.release);

    setDryWetMix(apvts.getRawParameterValue(paramId("mix"))->load() / 100.0f);
    setBypassed(apvts.getRawParameterValue(paramId("bypass"))->load() > 0.5f);
}

float MaximizerModule::applySoftClip(float x) {
    // Simple tanh-like soft clipper: x / (1 + |x|)
    // This squashes the signal towards 1.0/-1.0
    return x / (1.0f + std::abs(x));
}

void MaximizerModule::processInternal(juce::AudioBuffer<float>& buffer) {
    // 1. Gain boost is handled by the Limiter's threshold in JUCE's implementation 
    // actually, juce::dsp::Limiter threshold acts as both the threshold and the input boost 
    // to keep the output at 0dB.

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    limiter.process(context);

    // 2. Post-limiting processing: Soft clip and Ceiling
    float ceilingGain = juce::Decibels::decibelsToGain(params.ceiling);
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    if (params.softClip || params.ceiling != 0.0f) {
        for (int ch = 0; ch < numChannels; ++ch) {
            auto* data = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i) {
                float sample = data[i];
                if (params.softClip) {
                    sample = applySoftClip(sample);
                }
                data[i] = sample * ceilingGain;
            }
        }
    }
}

const std::string& MaximizerModule::getModuleId() const {
    return moduleId;
}

const std::string& MaximizerModule::getName() const {
    return name;
}

double MaximizerModule::getLatencySamples() const {
    // juce::dsp::Limiter has a fixed 5ms look-ahead latency
    // We should report this so the host can compensate.
    // Assuming 5ms as per standard juce implementation of Limiter.
    return 0.005 * limiter.getSampleRate();
}

} // namespace eeval
