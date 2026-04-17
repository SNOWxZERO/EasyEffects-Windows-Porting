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
    auto type = apvts.getRawParameterValue(paramId("type"));
    auto cutoff = apvts.getRawParameterValue(paramId("cutoff"));
    auto resonance = apvts.getRawParameterValue(paramId("resonance"));

    if (cutoff != nullptr && resonance != nullptr) {
        float freq = cutoff->load();
        float q = resonance->load();
        int filterType = (type != nullptr) ? (int)type->load() : 0;

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
    }

    auto mix = apvts.getRawParameterValue(paramId("mix"));
    if (mix != nullptr) setDryWetMix(mix->load() / 100.0f);
    auto byp = apvts.getRawParameterValue(paramId("bypass"));
    if (byp != nullptr) setBypassed(byp->load() > 0.5f);
}

const std::string& FilterModule::getModuleId() const { return moduleId; }
const std::string& FilterModule::getName() const { return name; }

} // namespace eeval
