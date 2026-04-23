#include "EqualizerModule.h"

namespace eeval {

EqualizerModule::EqualizerModule() {
    bands.resize(NUM_BANDS);
}

void EqualizerModule::prepare(const juce::dsp::ProcessSpec& spec) {
    currentSampleRate = spec.sampleRate;

    for (auto& band : bands) {
        for (auto& filter : band.filters) {
            filter.prepare(spec);
        }
    }
    prepareBuffers(spec);
}

void EqualizerModule::processInternal(juce::AudioBuffer<float>& buffer) {
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();

    for (int i = 0; i < NUM_BANDS; ++i) {
        auto& band = bands[i];
        if (!band.enabled) continue;

        for (int ch = 0; ch < juce::jmin(numChannels, 2); ++ch) {
            auto* channelData = buffer.getWritePointer(ch);
            juce::dsp::AudioBlock<float> singleChannelBlock(&channelData, 1, (size_t)numSamples);
            juce::dsp::ProcessContextReplacing<float> context(singleChannelBlock);
            band.filters[ch].process(context);
        }
    }
}

void EqualizerModule::reset() {
    for (auto& band : bands) {
        for (auto& filter : band.filters) {
            filter.reset();
        }
    }
}

void EqualizerModule::updateBandCoefficients(int i) {
    if (i < 0 || i >= NUM_BANDS) return;
    auto& band = bands[i];

    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> coeffs;
    float g = juce::Decibels::decibelsToGain(band.gain);

    switch (band.type) {
        case FilterType::Peak:
            coeffs.add(juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, band.freq, band.q, g));
            break;
        case FilterType::LowPass:
            coeffs.add(juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, band.freq, band.q));
            break;
        case FilterType::HighPass:
            coeffs.add(juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, band.freq, band.q));
            break;
        case FilterType::LowShelf:
            coeffs.add(juce::dsp::IIR::Coefficients<float>::makeLowShelf(currentSampleRate, band.freq, band.q, g));
            break;
        case FilterType::HighShelf:
            coeffs.add(juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, band.freq, band.q, g));
            break;
        case FilterType::Notch:
            coeffs.add(juce::dsp::IIR::Coefficients<float>::makeNotch(currentSampleRate, band.freq, band.q));
            break;
    }

    if (coeffs.size() > 0) {
        for (auto& filter : band.filters) {
            *filter.coefficients = *coeffs[0];
        }
    }
}

float EqualizerModule::getMagnitudeForFrequency(double frequency) const {
    double mag = 1.0;
    for (const auto& band : bands) {
        if (!band.enabled) continue;
        
        // Use the first channel's filter for magnitude calculation
        if (band.filters[0].coefficients != nullptr) {
            mag *= band.filters[0].coefficients->getMagnitudeForFrequency(frequency, currentSampleRate);
        }
    }
    return static_cast<float>(mag);
}

void EqualizerModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    for (int i = 0; i < NUM_BANDS; ++i) {
        auto bandPrefix = paramId("band" + std::to_string(i));
        
        auto& band = bands[i];
        band.enabled = loadFloat(bandPrefix + ".enabled", 0.0f) > 0.5f;
        band.type = static_cast<FilterType>((int)loadFloat(bandPrefix + ".type", 0.0f));
        band.gain = loadFloat(bandPrefix + ".gain", 0.0f);
        band.freq = loadFloat(bandPrefix + ".freq", 1000.0f);
        band.q = loadFloat(bandPrefix + ".q", 0.707f);

        updateBandCoefficients(i);
    }

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& EqualizerModule::getModuleId() const { return moduleId; }
const std::string& EqualizerModule::getName() const { return name; }

} // namespace eeval
