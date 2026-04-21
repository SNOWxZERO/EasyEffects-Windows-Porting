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
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    delayTimeMs = loadFloat(paramId("time_ms"), 200.0f);
    feedback = loadFloat(paramId("feedback"), 30.0f) / 100.0f;
    delayMix = loadFloat(paramId("delay_mix"), 50.0f) / 100.0f;

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& DelayModule::getModuleId() const { return moduleId; }
const std::string& DelayModule::getName() const { return name; }

} // namespace eeval
