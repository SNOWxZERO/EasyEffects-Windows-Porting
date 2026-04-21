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
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    compressorNode.setThreshold(loadFloat(paramId("threshold"), -10.0f));
    compressorNode.setRatio(loadFloat(paramId("ratio"), 3.0f));
    compressorNode.setAttack(loadFloat(paramId("attack"), 2.0f));
    compressorNode.setRelease(loadFloat(paramId("release"), 100.0f));

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& CompressorModule::getModuleId() const {
    return moduleId;
}

const std::string& CompressorModule::getName() const {
    return name;
}

} // namespace eeval
