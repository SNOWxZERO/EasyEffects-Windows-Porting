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
    auto thresh = apvts.getRawParameterValue(paramId("threshold"));
    auto ratio = apvts.getRawParameterValue(paramId("ratio"));
    auto attack = apvts.getRawParameterValue(paramId("attack"));
    auto release = apvts.getRawParameterValue(paramId("release"));

    if (thresh != nullptr) gateNode.setThreshold(thresh->load());
    if (ratio != nullptr) gateNode.setRatio(ratio->load());
    if (attack != nullptr) gateNode.setAttack(attack->load());
    if (release != nullptr) gateNode.setRelease(release->load());

    auto mix = apvts.getRawParameterValue(paramId("mix"));
    if (mix != nullptr) setDryWetMix(mix->load() / 100.0f);

    auto byp = apvts.getRawParameterValue(paramId("bypass"));
    if (byp != nullptr) setBypassed(byp->load() > 0.5f);
}

const std::string& GateModule::getModuleId() const { return moduleId; }
const std::string& GateModule::getName() const { return name; }

} // namespace eeval
