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
    auto thresh = apvts.getRawParameterValue(paramId("threshold"));
    auto release = apvts.getRawParameterValue(paramId("release"));

    if (thresh != nullptr) limiterNode.setThreshold(thresh->load());
    if (release != nullptr) limiterNode.setRelease(release->load());

    auto mix = apvts.getRawParameterValue(paramId("mix"));
    if (mix != nullptr) setDryWetMix(mix->load() / 100.0f);
    auto byp = apvts.getRawParameterValue(paramId("bypass"));
    if (byp != nullptr) setBypassed(byp->load() > 0.5f);
}

const std::string& LimiterModule::getModuleId() const { return moduleId; }
const std::string& LimiterModule::getName() const { return name; }

} // namespace eeval
