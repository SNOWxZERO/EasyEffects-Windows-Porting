#include "ConvolverModule.h"

namespace eeval {

ConvolverModule::ConvolverModule() {}

void ConvolverModule::prepare(const juce::dsp::ProcessSpec& spec) {
    convolverNode.prepare(spec);
    prepareBuffers(spec);
}

void ConvolverModule::processInternal(juce::AudioBuffer<float>& buffer) {
    if (!hasIRLoaded) return; // Pass through if no IR loaded

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    convolverNode.process(context);
}

void ConvolverModule::reset() {
    convolverNode.reset();
}

void ConvolverModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto mix = apvts.getRawParameterValue(paramId("mix"));
    if (mix != nullptr) setDryWetMix(mix->load() / 100.0f);
    
    auto byp = apvts.getRawParameterValue(paramId("bypass"));
    if (byp != nullptr) setBypassed(byp->load() > 0.5f);
}

void ConvolverModule::loadImpulseResponse(const juce::File& file) {
    if (file.existsAsFile()) {
        convolverNode.loadImpulseResponse(
            file,
            juce::dsp::Convolution::Stereo::yes,
            juce::dsp::Convolution::Trim::yes,
            0,
            juce::dsp::Convolution::Normalise::yes
        );
        hasIRLoaded = true;
    } else {
        hasIRLoaded = false;
    }
}

const std::string& ConvolverModule::getModuleId() const { return moduleId; }
const std::string& ConvolverModule::getName() const { return name; }

} // namespace eeval
