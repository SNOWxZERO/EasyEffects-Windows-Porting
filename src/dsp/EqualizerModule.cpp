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
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    for (int i = 0; i < NUM_BANDS; ++i) {
        auto bandPrefix = paramId("band" + std::to_string(i));
        float gain = loadFloat(bandPrefix + ".gain", 0.0f);
        float freq = loadFloat(bandPrefix + ".freq", 1000.0f);
        float q = loadFloat(bandPrefix + ".q", 0.707f);

        updateCoefficients(i, gain, freq, q);
    }

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& EqualizerModule::getModuleId() const { return moduleId; }
const std::string& EqualizerModule::getName() const { return name; }

} // namespace eeval
