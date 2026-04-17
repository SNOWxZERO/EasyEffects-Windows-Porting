#include "ExciterModule.h"
#include <cmath>

namespace eeval {

ExciterModule::ExciterModule() {
    waveShaper.functionToUse = [this](float x) {
        // Simple soft clipping added to the input taking 'harmonics' into account
        float drive = 1.0f + (harmonics * 10.0f);
        return std::tanh(x * drive) / std::tanh(drive);
    };
}

void ExciterModule::prepare(const juce::dsp::ProcessSpec& spec) {
    lpFilter.prepare(spec);
    hpFilter.prepare(spec);
    
    // Configure filter types
    lpFilter.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    hpFilter.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    
    waveShaper.prepare(spec);
    
    lowBandBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    highBandBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    
    prepareBuffers(spec);
}

void ExciterModule::processInternal(juce::AudioBuffer<float>& buffer) {
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();
    
    // Copy input to split bands
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
    
    // Apply saturator to high band ONLY
    waveShaper.process(hpContext);

    // Recombine: Main buffer = LowBand + (HighBand * amount)
    for (int ch = 0; ch < numChannels; ++ch) {
        auto* out = buffer.getWritePointer(ch);
        const auto* low = lowBandBuffer.getReadPointer(ch);
        const auto* high = highBandBuffer.getReadPointer(ch);
        
        for (int i = 0; i < numSamples; ++i) {
            out[i] = low[i] + (high[i] * amount);
        }
    }
}

void ExciterModule::reset() {
    lpFilter.reset();
    hpFilter.reset();
    waveShaper.reset();
}

void ExciterModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto amtParam = apvts.getRawParameterValue(paramId("amount"));
    auto harmParam = apvts.getRawParameterValue(paramId("harmonics"));
    auto cutParam = apvts.getRawParameterValue(paramId("cutoff"));
    
    if (amtParam != nullptr) amount = amtParam->load();
    if (harmParam != nullptr) harmonics = harmParam->load();
    if (cutParam != nullptr) {
        lpFilter.setCutoffFrequency(cutParam->load());
        hpFilter.setCutoffFrequency(cutParam->load());
    }
    
    auto mix = apvts.getRawParameterValue(paramId("mix"));
    if (mix != nullptr) setDryWetMix(mix->load() / 100.0f);
    
    auto byp = apvts.getRawParameterValue(paramId("bypass"));
    if (byp != nullptr) setBypassed(byp->load() > 0.5f);
}

const std::string& ExciterModule::getModuleId() const { return moduleId; }
const std::string& ExciterModule::getName() const { return name; }

} // namespace eeval
