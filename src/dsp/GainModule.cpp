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
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    gainNode.setGainDecibels(loadFloat(paramId("level"), 0.0f));

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& GainModule::getModuleId() const {
    return moduleId;
}

const std::string& GainModule::getName() const {
    return name;
}

} // namespace eeval
