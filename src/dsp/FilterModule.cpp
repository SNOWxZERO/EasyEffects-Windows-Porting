#include "FilterModule.h"

namespace eeval {

FilterModule::FilterModule() {}

void FilterModule::prepare(const juce::dsp::ProcessSpec& spec) {
    currentSampleRate = spec.sampleRate;
    filterNode.prepare(spec);
    prepareBuffers(spec);
}

void FilterModule::processInternal(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filterNode.process(context);
}

void FilterModule::reset() { filterNode.reset(); }

void FilterModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    float freq = loadFloat(paramId("cutoff"), 1000.0f);
    float q = loadFloat(paramId("resonance"), 0.707f);
    int filterType = (int)loadFloat(paramId("type"), 0.0f);

    // Clamp frequency to valid range
    freq = juce::jlimit(20.0f, (float)(currentSampleRate * 0.49), freq);
    q = juce::jmax(0.1f, q);

    if (filterType == 0) {
        *filterNode.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
            currentSampleRate, freq, q);
    } else {
        *filterNode.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
            currentSampleRate, freq, q);
    }

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& FilterModule::getModuleId() const { return moduleId; }
const std::string& FilterModule::getName() const { return name; }

} // namespace eeval
