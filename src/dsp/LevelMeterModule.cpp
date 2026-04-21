#include "LevelMeterModule.h"

namespace eeval {

LevelMeterModule::LevelMeterModule() {}

void LevelMeterModule::prepare(const juce::dsp::ProcessSpec& spec) {
    // Determine decay factors based on sample rate (approx 300ms fallback for RMS, 100ms for peak)
    rmsDecay = std::exp(-1.0f / (float)(spec.sampleRate * 0.3f));
    peakDecay = std::exp(-1.0f / (float)(spec.sampleRate * 0.1f));
    prepareBuffers(spec);
}

void LevelMeterModule::processInternal(juce::AudioBuffer<float>& buffer) {
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    if (numSamples == 0 || numChannels == 0) return;

    // Calculate block peak and sum of squares
    float blockPeakLevel = 0.0f;
    float sumSquares = 0.0f;

    // Analyze output but do not modify buffer
    for (int ch = 0; ch < numChannels; ++ch) {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            float sample = std::abs(data[i]);
            if (sample > blockPeakLevel) blockPeakLevel = sample;
            sumSquares += (sample * sample);
        }
    }

    float blockRmsLevel = std::sqrt(sumSquares / (float)(numSamples * numChannels));

    // Apply smoothing
    float previousRms = currentRms.load();
    float previousPeak = currentPeak.load();

    float blockRmsDb = juce::Decibels::gainToDecibels(blockRmsLevel, -100.0f);
    float blockPeakDb = juce::Decibels::gainToDecibels(blockPeakLevel, -100.0f);

    float smoothedRms = (blockRmsDb > previousRms) ? blockRmsDb : previousRms * rmsDecay + blockRmsDb * (1.0f - rmsDecay);
    float smoothedPeak = (blockPeakDb > previousPeak) ? blockPeakDb : previousPeak * peakDecay + blockPeakDb * (1.0f - peakDecay);

    currentRms.store(smoothedRms);
    currentPeak.store(smoothedPeak);
}

void LevelMeterModule::reset() {
    currentRms.store(-100.0f);
    currentPeak.store(-100.0f);
}

void LevelMeterModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& LevelMeterModule::getModuleId() const { return moduleId; }
const std::string& LevelMeterModule::getName() const { return name; }

} // namespace eeval
