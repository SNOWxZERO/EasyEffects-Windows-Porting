#include "ReverbModule.h"

namespace eeval {

ReverbModule::ReverbModule() {}

void ReverbModule::prepare(const juce::dsp::ProcessSpec& spec) {
    reverbNode.prepare(spec);
    prepareBuffers(spec);
}

void ReverbModule::processInternal(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverbNode.process(context);
}

void ReverbModule::reset() { reverbNode.reset(); }

void ReverbModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto roomSize = apvts.getRawParameterValue(paramId("room_size"));
    auto damping = apvts.getRawParameterValue(paramId("damping"));
    auto wet = apvts.getRawParameterValue(paramId("wet"));
    auto dry = apvts.getRawParameterValue(paramId("dry"));
    auto width = apvts.getRawParameterValue(paramId("width"));

    if (roomSize != nullptr) reverbParams.roomSize = roomSize->load();
    if (damping != nullptr) reverbParams.damping = damping->load();
    if (wet != nullptr) reverbParams.wetLevel = wet->load();
    if (dry != nullptr) reverbParams.dryLevel = dry->load();
    if (width != nullptr) reverbParams.width = width->load();

    reverbNode.setParameters(reverbParams);

    auto mix = apvts.getRawParameterValue(paramId("mix"));
    if (mix != nullptr) setDryWetMix(mix->load() / 100.0f);
    auto byp = apvts.getRawParameterValue(paramId("bypass"));
    if (byp != nullptr) setBypassed(byp->load() > 0.5f);
}

const std::string& ReverbModule::getModuleId() const { return moduleId; }
const std::string& ReverbModule::getName() const { return name; }

} // namespace eeval
