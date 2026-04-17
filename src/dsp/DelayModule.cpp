#include "DelayModule.h"

namespace eeval {

DelayModule::DelayModule() {}

void DelayModule::prepare(const juce::dsp::ProcessSpec& spec) {
    currentSampleRate = spec.sampleRate;
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples((int)(MAX_DELAY_MS * spec.sampleRate / 1000.0));
    prepareBuffers(spec);
}

void DelayModule::processInternal(juce::AudioBuffer<float>& buffer) {
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();

    float delaySamples = (float)(delayTimeMs * currentSampleRate / 1000.0);
    delayLine.setDelay(delaySamples);

    for (int ch = 0; ch < numChannels; ++ch) {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            float inputSample = data[i];
            float delayedSample = delayLine.popSample(ch);
            delayLine.pushSample(ch, inputSample + delayedSample * feedback);
            data[i] = inputSample * (1.0f - delayMix) + delayedSample * delayMix;
        }
    }
}

void DelayModule::reset() { delayLine.reset(); }

void DelayModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto time = apvts.getRawParameterValue(paramId("time_ms"));
    auto fb = apvts.getRawParameterValue(paramId("feedback"));
    auto dmix = apvts.getRawParameterValue(paramId("delay_mix"));

    if (time != nullptr) delayTimeMs = time->load();
    if (fb != nullptr) feedback = fb->load() / 100.0f;
    if (dmix != nullptr) delayMix = dmix->load() / 100.0f;

    auto mix = apvts.getRawParameterValue(paramId("mix"));
    if (mix != nullptr) setDryWetMix(mix->load() / 100.0f);
    auto byp = apvts.getRawParameterValue(paramId("bypass"));
    if (byp != nullptr) setBypassed(byp->load() > 0.5f);
}

const std::string& DelayModule::getModuleId() const { return moduleId; }
const std::string& DelayModule::getName() const { return name; }

} // namespace eeval
