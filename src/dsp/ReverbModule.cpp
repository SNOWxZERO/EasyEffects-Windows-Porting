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
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    reverbParams.roomSize = loadFloat(paramId("room_size"), 0.5f);
    reverbParams.damping = loadFloat(paramId("damping"), 0.5f);
    reverbParams.wetLevel = loadFloat(paramId("wet"), 0.33f);
    reverbParams.dryLevel = loadFloat(paramId("dry"), 0.4f);
    reverbParams.width = loadFloat(paramId("width"), 1.0f);

    reverbNode.setParameters(reverbParams);

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& ReverbModule::getModuleId() const { return moduleId; }
const std::string& ReverbModule::getName() const { return name; }

} // namespace eeval
