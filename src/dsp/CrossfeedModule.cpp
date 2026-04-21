#include "CrossfeedModule.h"
#include <cmath>

namespace eeval {

CrossfeedModule::CrossfeedModule() {
}

void CrossfeedModule::prepare(const juce::dsp::ProcessSpec& spec) {
    currentSampleRate = spec.sampleRate;
    
    directChainL.prepare(spec);
    directChainR.prepare(spec);
    crossChainL.prepare(spec);
    crossChainR.prepare(spec);
    
    updateFilters();
    prepareBuffers(spec);
}

void CrossfeedModule::reset() {
    directChainL.reset();
    directChainR.reset();
    crossChainL.reset();
    crossChainR.reset();
}

void CrossfeedModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    float newFcut = apvts.getRawParameterValue(paramId("fcut"))->load();
    float newFeed = apvts.getRawParameterValue(paramId("feed"))->load();

    if (newFcut != params.fcut || newFeed != params.feed) {
        params.fcut = newFcut;
        params.feed = newFeed;
        updateFilters();
    }

    setDryWetMix(apvts.getRawParameterValue(paramId("mix"))->load() / 100.0f);
    setBypassed(apvts.getRawParameterValue(paramId("bypass"))->load() > 0.5f);
}

void CrossfeedModule::updateFilters() {
    // k is the crosstalk gain (linear)
    float k = juce::Decibels::decibelsToGain(-params.feed);
    
    // Simple 1st order low-pass for crosstalk
    // H_cross(s) = k / (1 + s*tau)
    auto crossCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, params.fcut, 0.707f); 
    // Note: makeLowPass creates 2nd order. For 1st order I might need to manual.
    // JUCE doesn't have 1st order makeLowPass helper in some versions, but we can use 2nd order with Q=0.5 for a smooth response
    // or just calculate 1st order: y[n] = a0*x[n] + a1*x[n-1] - b1*y[n-1]
    
    // Direct path: H_direct(s) = (1 - k + s*tau) / (1 + s*tau)
    // This is a high-shelf with gain < 1 at DC.
    
    // For simplicity and stability, we use JUCE's 2nd order wrappers:
    *directChainL.get<0>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, params.fcut, 0.707f, k);
    *directChainR.get<0>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, params.fcut, 0.707f, k);
    
    // Cross path should be low-pass with gain k
    auto cCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, params.fcut, 0.707f);
    for (auto& c : cCoeffs->coefficients) c *= k;
    
    *crossChainL.get<0>().coefficients = *cCoeffs;
    *crossChainR.get<0>().coefficients = *cCoeffs;
}

void CrossfeedModule::processInternal(juce::AudioBuffer<float>& buffer) {
    auto numSamples = buffer.getNumSamples();
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    if (!right) return;

    for (int i = 0; i < numSamples; ++i) {
        float lIn = left[i];
        float rIn = right[i];

        // Process Direct
        float lDirect = directChainL.get<0>().processSample(lIn);
        float rDirect = directChainR.get<0>().processSample(rIn);

        // Process Cross
        float lCross = crossChainL.get<0>().processSample(lIn);
        float rCross = crossChainR.get<0>().processSample(rIn);

        // Mix: Left gets Direct Left + Cross Right
        left[i] = lDirect + rCross;
        // Right gets Direct Right + Cross Left
        right[i] = rDirect + lCross;
    }
}

const std::string& CrossfeedModule::getModuleId() const {
    return moduleId;
}

const std::string& CrossfeedModule::getName() const {
    return name;
}

} // namespace eeval
