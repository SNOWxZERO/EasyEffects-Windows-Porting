#include "DeesserModule.h"

namespace eeval {

DeesserModule::DeesserModule() {}

void DeesserModule::prepare(const juce::dsp::ProcessSpec& spec) {
    lpFilter.prepare(spec);
    hpFilter.prepare(spec);
    
    lpFilter.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    hpFilter.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    
    compressorNode.prepare(spec);
    
    lowBandBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    highBandBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    
    prepareBuffers(spec);
}

void DeesserModule::processInternal(juce::AudioBuffer<float>& buffer) {
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();
    
    for (int ch = 0; ch < numChannels; ++ch) {
        lowBandBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        highBandBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    }
    
    juce::dsp::AudioBlock<float> lpBlock(lowBandBuffer);
    juce::dsp::ProcessContextReplacing<float> lpContext(lpBlock);
    lpFilter.process(lpContext);

    juce::dsp::AudioBlock<float> hpBlock(highBandBuffer);
    juce::dsp::ProcessContextReplacing<float> hpContext(hpBlock);
    hpFilter.process(hpContext);
    
    // Apply compressor to HIGH band ONLY (sibilance)
    compressorNode.process(hpContext);

    // Recombine: Main buffer = LowBand + Squashed HighBand
    for (int ch = 0; ch < numChannels; ++ch) {
        auto* out = buffer.getWritePointer(ch);
        const auto* low = lowBandBuffer.getReadPointer(ch);
        const auto* high = highBandBuffer.getReadPointer(ch);
        
        for (int i = 0; i < numSamples; ++i) {
            out[i] = low[i] + high[i];
        }
    }
}

void DeesserModule::reset() {
    lpFilter.reset();
    hpFilter.reset();
    compressorNode.reset();
}

void DeesserModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto threshParam = apvts.getRawParameterValue(paramId("threshold"));
    auto ratioParam = apvts.getRawParameterValue(paramId("ratio"));
    auto freqParam = apvts.getRawParameterValue(paramId("frequency"));
    
    if (threshParam != nullptr) compressorNode.setThreshold(threshParam->load());
    if (ratioParam != nullptr) compressorNode.setRatio(ratioParam->load());
    if (freqParam != nullptr) {
        lpFilter.setCutoffFrequency(freqParam->load());
        hpFilter.setCutoffFrequency(freqParam->load());
    }
    
    auto mix = apvts.getRawParameterValue(paramId("mix"));
    if (mix != nullptr) setDryWetMix(mix->load() / 100.0f);
    
    auto byp = apvts.getRawParameterValue(paramId("bypass"));
    if (byp != nullptr) setBypassed(byp->load() > 0.5f);
}

const std::string& DeesserModule::getModuleId() const { return moduleId; }
const std::string& DeesserModule::getName() const { return name; }

} // namespace eeval
