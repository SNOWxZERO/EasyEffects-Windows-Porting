#include "GainModule.h"

namespace eeval {

GainModule::GainModule() {
}

void GainModule::prepare(const juce::dsp::ProcessSpec& spec) {
    gainNode.prepare(spec);
    gainNode.setRampDurationSeconds(0.05);
    prepareBuffers(spec);
}

void GainModule::processInternal(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    gainNode.process(context);
}

void GainModule::reset() {
    gainNode.reset();
}

void GainModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto level = apvts.getRawParameterValue(paramId("level"));
    if (level != nullptr)
        gainNode.setGainDecibels(level->load());

    auto mix = apvts.getRawParameterValue(paramId("mix"));
    if (mix != nullptr)
        setDryWetMix(mix->load() / 100.0f);

    auto byp = apvts.getRawParameterValue(paramId("bypass"));
    if (byp != nullptr)
        setBypassed(byp->load() > 0.5f);
}

const std::string& GainModule::getModuleId() const {
    return moduleId;
}

const std::string& GainModule::getName() const {
    return name;
}

} // namespace eeval
