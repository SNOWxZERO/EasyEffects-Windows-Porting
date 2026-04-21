#include "BassEnhancerModule.h"
#include <cmath>

namespace eeval {

BassEnhancerModule::BassEnhancerModule() {
    waveShaper.functionToUse = [this](float x) {
        float drive = 1.0f + (harmonics * 10.0f);
        return std::tanh(x * drive) / std::tanh(drive);
    };
}

void BassEnhancerModule::prepare(const juce::dsp::ProcessSpec& spec) {
    lpFilter.prepare(spec);
    hpFilter.prepare(spec);
    
    lpFilter.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    hpFilter.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    
    waveShaper.prepare(spec);
    
    lowBandBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    highBandBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    
    prepareBuffers(spec);
}

void BassEnhancerModule::processInternal(juce::AudioBuffer<float>& buffer) {
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
    
    // Apply saturator to LOW band ONLY
    waveShaper.process(lpContext);

    // Recombine: Main buffer = HighBand + (LowBand * amount)
    for (int ch = 0; ch < numChannels; ++ch) {
        auto* out = buffer.getWritePointer(ch);
        const auto* low = lowBandBuffer.getReadPointer(ch);
        const auto* high = highBandBuffer.getReadPointer(ch);
        
        for (int i = 0; i < numSamples; ++i) {
            out[i] = high[i] + (low[i] * amount);
        }
    }
}

void BassEnhancerModule::reset() {
    lpFilter.reset();
    hpFilter.reset();
    waveShaper.reset();
}

void BassEnhancerModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    amount = loadFloat(paramId("amount"), 0.5f);
    harmonics = loadFloat(paramId("harmonics"), 0.1f);
    float cutoff = loadFloat(paramId("cutoff"), 120.0f);
    
    lpFilter.setCutoffFrequency(cutoff);
    hpFilter.setCutoffFrequency(cutoff);

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& BassEnhancerModule::getModuleId() const { return moduleId; }
const std::string& BassEnhancerModule::getName() const { return name; }

} // namespace eeval
