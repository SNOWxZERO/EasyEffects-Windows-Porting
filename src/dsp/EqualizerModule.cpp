#include "EqualizerModule.h"

namespace eeval {

EqualizerModule::EqualizerModule() {
    filters.resize(NUM_BANDS);
}

void EqualizerModule::prepare(const juce::dsp::ProcessSpec& spec) {
    currentSampleRate = spec.sampleRate;

    // Pre-allocate all filters
    filters.resize(NUM_BANDS);
    for (auto& bandFilters : filters) {
        for (auto& filter : bandFilters) {
            filter.prepare(spec);
        }
    }
    prepareBuffers(spec);
}

void EqualizerModule::processInternal(juce::AudioBuffer<float>& buffer) {
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();

    for (int band = 0; band < NUM_BANDS; ++band) {
        for (int ch = 0; ch < juce::jmin(numChannels, 2); ++ch) {
            auto* channelData = buffer.getWritePointer(ch);
            juce::dsp::AudioBlock<float> singleChannelBlock(&channelData, 1, (size_t)numSamples);
            juce::dsp::ProcessContextReplacing<float> context(singleChannelBlock);
            filters[band][ch].process(context);
        }
    }
}

void EqualizerModule::reset() {
    for (auto& bandFilters : filters) {
        for (auto& filter : bandFilters) {
            filter.reset();
        }
    }
}

void EqualizerModule::updateCoefficients(int bandIndex, float gain, float freq, float q) {
    if (bandIndex < 0 || bandIndex >= NUM_BANDS) return;

    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> coeffs;

    if (bandIndex == 0) {
        // Low shelf
        coeffs.add(juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            currentSampleRate, freq, q, juce::Decibels::decibelsToGain(gain)));
    } else if (bandIndex == NUM_BANDS - 1) {
        // High shelf
        coeffs.add(juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            currentSampleRate, freq, q, juce::Decibels::decibelsToGain(gain)));
    } else {
        // Peak/Bell
        coeffs.add(juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            currentSampleRate, freq, q, juce::Decibels::decibelsToGain(gain)));
    }

    if (coeffs.size() > 0) {
        for (auto& filter : filters[bandIndex]) {
            *filter.coefficients = *coeffs[0];
        }
    }
}

void EqualizerModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    for (int i = 0; i < NUM_BANDS; ++i) {
        auto bandPrefix = paramId("band" + std::to_string(i));
        auto gain = apvts.getRawParameterValue(bandPrefix + ".gain");
        auto freq = apvts.getRawParameterValue(bandPrefix + ".freq");
        auto q = apvts.getRawParameterValue(bandPrefix + ".q");

        if (gain != nullptr && freq != nullptr && q != nullptr) {
            updateCoefficients(i, gain->load(), freq->load(), q->load());
        }
    }

    auto mix = apvts.getRawParameterValue(paramId("mix"));
    if (mix != nullptr) setDryWetMix(mix->load() / 100.0f);

    auto byp = apvts.getRawParameterValue(paramId("bypass"));
    if (byp != nullptr) setBypassed(byp->load() > 0.5f);
}

const std::string& EqualizerModule::getModuleId() const { return moduleId; }
const std::string& EqualizerModule::getName() const { return name; }

} // namespace eeval
