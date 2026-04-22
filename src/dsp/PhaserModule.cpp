#include "PhaserModule.h"

namespace eeval {

PhaserModule::PhaserModule() {}

void PhaserModule::prepare(const juce::dsp::ProcessSpec& spec) {
    phaserNode.prepare(spec);
    prepareBuffers(spec);
}

void PhaserModule::reset() {
    phaserNode.reset();
}

void PhaserModule::processInternal(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    phaserNode.process(context);
}

void PhaserModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    phaserNode.setRate(loadFloat(paramId("rate"), 0.5f));
    phaserNode.setDepth(loadFloat(paramId("depth"), 0.5f));
    phaserNode.setFeedback(loadFloat(paramId("feedback"), 0.5f));
    phaserNode.setCentreFrequency(loadFloat(paramId("centre_freq"), 1000.0f));
    
    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& PhaserModule::getModuleId() const { return moduleId; }
const std::string& PhaserModule::getName() const { return name; }

} // namespace eeval
