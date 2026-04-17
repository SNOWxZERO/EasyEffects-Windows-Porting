#include "CompressorModule.h"

namespace eeval {

CompressorModule::CompressorModule() {
}

void CompressorModule::prepare(const juce::dsp::ProcessSpec& spec) {
    compressorNode.prepare(spec);
    prepareBuffers(spec);
}

void CompressorModule::processInternal(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    compressorNode.process(context);
}

void CompressorModule::reset() {
    compressorNode.reset();
}

void CompressorModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto thresh = apvts.getRawParameterValue(paramId("threshold"));
    auto ratio = apvts.getRawParameterValue(paramId("ratio"));
    auto attack = apvts.getRawParameterValue(paramId("attack"));
    auto release = apvts.getRawParameterValue(paramId("release"));

    if (thresh != nullptr) compressorNode.setThreshold(thresh->load());
    if (ratio != nullptr) compressorNode.setRatio(ratio->load());
    if (attack != nullptr) compressorNode.setAttack(attack->load());
    if (release != nullptr) compressorNode.setRelease(release->load());

    auto mix = apvts.getRawParameterValue(paramId("mix"));
    if (mix != nullptr)
        setDryWetMix(mix->load() / 100.0f);

    auto byp = apvts.getRawParameterValue(paramId("bypass"));
    if (byp != nullptr)
        setBypassed(byp->load() > 0.5f);
}

const std::string& CompressorModule::getModuleId() const {
    return moduleId;
}

const std::string& CompressorModule::getName() const {
    return name;
}

} // namespace eeval
