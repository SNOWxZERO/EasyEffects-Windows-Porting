#include "ChorusModule.h"

namespace eeval {

ChorusModule::ChorusModule() {}

void ChorusModule::prepare(const juce::dsp::ProcessSpec& spec) {
    chorusNode.prepare(spec);
    prepareBuffers(spec);
}

void ChorusModule::reset() {
    chorusNode.reset();
}

void ChorusModule::processInternal(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    chorusNode.process(context);
}

void ChorusModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    chorusNode.setRate(loadFloat(paramId("rate"), 1.0f));
    chorusNode.setDepth(loadFloat(paramId("depth"), 0.5f));
    chorusNode.setFeedback(loadFloat(paramId("feedback"), 0.0f));
    chorusNode.setCentreDelay(loadFloat(paramId("centre_delay"), 7.0f));
    chorusNode.setMix(loadFloat(paramId("chorus_mix"), 0.5f));

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& ChorusModule::getModuleId() const { return moduleId; }
const std::string& ChorusModule::getName() const { return name; }

} // namespace eeval
