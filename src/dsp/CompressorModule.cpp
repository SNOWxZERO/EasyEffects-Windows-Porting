#include "CompressorModule.h"

namespace eeval {

CompressorModule::CompressorModule() {
}

void CompressorModule::prepare(const juce::dsp::ProcessSpec& spec) {
    compressorNode.prepare(spec);
}

void CompressorModule::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    if (isBypassed()) return;
    compressorNode.process(context);
}

void CompressorModule::reset() {
    compressorNode.reset();
}

void CompressorModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto thresh = apvts.getRawParameterValue("comp_threshold");
    auto ratio = apvts.getRawParameterValue("comp_ratio");
    auto attack = apvts.getRawParameterValue("comp_attack");
    auto release = apvts.getRawParameterValue("comp_release");

    if (thresh != nullptr) compressorNode.setThreshold(thresh->load());
    if (ratio != nullptr) compressorNode.setRatio(ratio->load());
    if (attack != nullptr) compressorNode.setAttack(attack->load());
    if (release != nullptr) compressorNode.setRelease(release->load());
}

const std::string& CompressorModule::getName() const {
    return name;
}

} // namespace eeval
