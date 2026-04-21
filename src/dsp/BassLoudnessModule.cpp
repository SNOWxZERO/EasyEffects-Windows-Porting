#include "BassLoudnessModule.h"
#include <cmath>

namespace eeval {

BassLoudnessModule::BassLoudnessModule() {
}

void BassLoudnessModule::prepare(const juce::dsp::ProcessSpec& spec) {
    currentSampleRate = spec.sampleRate;
    
    shelfL.prepare(spec);
    shelfR.prepare(spec);
    
    smoothedLoudnessGain.reset(spec.sampleRate, 0.05);
    smoothedOutputGain.reset(spec.sampleRate, 0.05);
    
    updateFilters();
    prepareBuffers(spec);
}

void BassLoudnessModule::reset() {
    shelfL.reset();
    shelfR.reset();
}

void BassLoudnessModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    params.loudness = apvts.getRawParameterValue(paramId("loudness"))->load();
    params.output = apvts.getRawParameterValue(paramId("output"))->load();
    params.link = apvts.getRawParameterValue(paramId("link"))->load() > 0.5f;

    // In the MDA version, 'loudness' is often a drive parameter.
    // We update the filters when loudness changes.
    updateFilters();
    
    smoothedOutputGain.setTargetValue(juce::Decibels::decibelsToGain(params.output));

    setDryWetMix(apvts.getRawParameterValue(paramId("mix"))->load() / 100.0f);
    setBypassed(apvts.getRawParameterValue(paramId("bypass"))->load() > 0.5f);
}

void BassLoudnessModule::updateFilters() {
    // MDA Loudness typical cutoff is around 180 Hz
    const float cutoff = 180.0f;
    const float q = 0.707f;
    
    // The 'loudness' parameter (0 to 30dB or similar) controls the shelf gain
    float gain = juce::Decibels::decibelsToGain(params.loudness);
    
    auto coeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(currentSampleRate, cutoff, q, gain);
    
    *shelfL.get<0>().coefficients = *coeffs;
    *shelfR.get<0>().coefficients = *coeffs;
}

void BassLoudnessModule::processInternal(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    
    // Process L and R through the shelf filters
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = (buffer.getNumChannels() > 1) ? block.getSingleChannelBlock(1) : leftBlock;

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    shelfL.process(leftContext);
    if (buffer.getNumChannels() > 1) {
        shelfR.process(rightContext);
    }

    // Apply output gain
    float outGain = smoothedOutputGain.getNextValue();
    if (outGain != 1.0f) {
        buffer.applyGain(outGain);
    }
}

const std::string& BassLoudnessModule::getModuleId() const {
    return moduleId;
}

const std::string& BassLoudnessModule::getName() const {
    return name;
}

} // namespace eeval
