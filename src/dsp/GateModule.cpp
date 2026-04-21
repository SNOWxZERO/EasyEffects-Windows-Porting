#include "GateModule.h"

namespace eeval {

GateModule::GateModule() {}

void GateModule::prepare(const juce::dsp::ProcessSpec& spec) {
    gateNode.prepare(spec);
    prepareBuffers(spec);
}

void GateModule::processInternal(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    gateNode.process(context);
}

void GateModule::reset() {
    gateNode.reset();
}

void GateModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    gateNode.setThreshold(loadFloat(paramId("threshold"), -40.0f));
    gateNode.setRatio(loadFloat(paramId("ratio"), 10.0f));
    gateNode.setAttack(loadFloat(paramId("attack"), 1.0f));
    gateNode.setRelease(loadFloat(paramId("release"), 100.0f));

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& GateModule::getModuleId() const { return moduleId; }
const std::string& GateModule::getName() const { return name; }

} // namespace eeval
