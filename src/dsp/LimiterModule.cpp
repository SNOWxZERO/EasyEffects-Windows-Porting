#include "LimiterModule.h"

namespace eeval {

LimiterModule::LimiterModule() {}

void LimiterModule::prepare(const juce::dsp::ProcessSpec& spec) {
    limiterNode.prepare(spec);
    prepareBuffers(spec);
}

void LimiterModule::processInternal(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    limiterNode.process(context);
}

void LimiterModule::reset() { limiterNode.reset(); }

void LimiterModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    limiterNode.setThreshold(loadFloat(paramId("threshold"), -1.0f));
    limiterNode.setRelease(loadFloat(paramId("release"), 100.0f));

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& LimiterModule::getModuleId() const { return moduleId; }
const std::string& LimiterModule::getName() const { return name; }

} // namespace eeval
