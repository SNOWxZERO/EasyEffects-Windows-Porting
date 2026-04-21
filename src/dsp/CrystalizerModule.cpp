#include "CrystalizerModule.h"
#include <cmath>

namespace eeval {

CrystalizerModule::CrystalizerModule() {
    for (auto& ch : lastSamples) ch.fill(0.0f);
    for (auto& ch : lastProcessedSamples) ch.fill(0.0f);
}

void CrystalizerModule::prepare(const juce::dsp::ProcessSpec& spec) {
    currentSampleRate = spec.sampleRate;
    
    // We need 4 filters per channel for a 3-band split
    // Since I defined only 2 in the header, I'll update the header or use local ones
    // Actually I will update the header to be cleaner.
    
    cross1L.prepare(spec); cross1R.prepare(spec);
    cross2L.prepare(spec); cross2R.prepare(spec);
    
    cross1L.setCutoffFrequency(lowMidFreq);
    cross1R.setCutoffFrequency(lowMidFreq);
    cross2L.setCutoffFrequency(midHighFreq);
    cross2R.setCutoffFrequency(midHighFreq);
    
    prepareBuffers(spec);
}

void CrystalizerModule::reset() {
    cross1L.reset();
    cross1R.reset();
    cross2L.reset();
    cross2R.reset();
    for (auto& ch : lastSamples) ch.fill(0.0f);
    for (auto& ch : lastProcessedSamples) ch.fill(0.0f);
}

void CrystalizerModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    params.intensityLow = apvts.getRawParameterValue(paramId("intensity_low"))->load();
    params.intensityMid = apvts.getRawParameterValue(paramId("intensity_mid"))->load();
    params.intensityHigh = apvts.getRawParameterValue(paramId("intensity_high"))->load();
    params.adaptiveIntensity = apvts.getRawParameterValue(paramId("adaptive_intensity"))->load() > 0.5f;

    setDryWetMix(apvts.getRawParameterValue(paramId("mix"))->load() / 100.0f);
    setBypassed(apvts.getRawParameterValue(paramId("bypass"))->load() > 0.5f);
}

float CrystalizerModule::processBand(int channel, int band, float x_n, float intensity) {
    if (intensity == 0.0f) return x_n;

    float x_n_1 = lastSamples[channel][band];
    float x_n_2 = lastProcessedSamples[channel][band];

    // Second derivative estimate: d2 = x[n] - 2*x[n-1] + x[n-2]
    float d2 = x_n - 2.0f * x_n_1 + x_n_2;
    
    // Update state
    lastProcessedSamples[channel][band] = x_n_1;
    lastSamples[channel][band] = x_n;

    // Sharpen
    return x_n - (intensity * d2);
}

void CrystalizerModule::processInternal(juce::AudioBuffer<float>& buffer) {
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i) {
        for (int ch = 0; ch < numChannels; ++ch) {
            float x = buffer.getSample(ch, i);
            
            auto& c1 = (ch == 0) ? cross1L : cross1R;
            auto& c2 = (ch == 0) ? cross2L : cross2R;

            // Linkwitz-Riley in JUCE doesn't return both automatically.
            // We use a trick: the sum is flat. But we need the high path for the next split.
            
            // Split 1: Low-Mid (Low-pass)
            c1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
            float low = c1.processSample(0, x);
            float midHighBase = x - low; // Approximate high path
            
            // Split 2: Mid vs High (sub-splitting the high path)
            c2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
            float mid = c2.processSample(0, midHighBase);
            float high = midHighBase - mid; // Approximate high path

            // Apply Crystalizer logic to each band
            float processedLow = processBand(ch, 0, low, params.intensityLow);
            float processedMid = processBand(ch, 1, mid, params.intensityMid);
            float processedHigh = processBand(ch, 2, high, params.intensityHigh);

            // Recombine
            buffer.setSample(ch, i, processedLow + processedMid + processedHigh);
        }
    }
}

const std::string& CrystalizerModule::getModuleId() const {
    return moduleId;
}

const std::string& CrystalizerModule::getName() const {
    return name;
}

} // namespace eeval
