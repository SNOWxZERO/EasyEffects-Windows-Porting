#include "MultibandCompressorModule.h"

namespace eeval {

MultibandCompressorModule::MultibandCompressorModule() {
}

void MultibandCompressorModule::prepare(const juce::dsp::ProcessSpec& spec) {
    splitter.prepare(spec);
    for (int i = 0; i < 3; ++i) {
        bands[i].prepare(spec);
        bandBuffers[i].setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    }
    prepareBuffers(spec);
}

void MultibandCompressorModule::reset() {
    splitter.reset();
    for (int i = 0; i < 3; ++i) {
        bands[i].reset();
    }
}

void MultibandCompressorModule::processInternal(juce::AudioBuffer<float>& buffer) {
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    // 1. Split input into 3 bands
    for (int ch = 0; ch < numChannels; ++ch) {
        auto* input = buffer.getReadPointer(ch);
        auto* low = bandBuffers[0].getWritePointer(ch);
        auto* mid = bandBuffers[1].getWritePointer(ch);
        auto* high = bandBuffers[2].getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i) {
            splitter.processSample(ch, input[i], low[i], mid[i], high[i]);
        }
    }

    // 2. Process each band independently
    for (int i = 0; i < 3; ++i) {
        juce::dsp::AudioBlock<float> block(bandBuffers[i]);
        // Limit block to current sample count
        auto subBlock = block.getSubBlock(0, (size_t)numSamples);
        juce::dsp::ProcessContextReplacing<float> context(subBlock);
        
        bands[i].compressor.process(context);
        bands[i].makeupGain.process(context);
    }

    // 3. Recombine
    buffer.clear();
    for (int i = 0; i < 3; ++i) {
        for (int ch = 0; ch < numChannels; ++ch) {
            buffer.addFrom(ch, 0, bandBuffers[i], ch, 0, numSamples);
        }
    }
}

void MultibandCompressorModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    // Crossovers
    lowMidFreq = loadFloat(paramId("crossover_low"), 200.0f);
    midHighFreq = loadFloat(paramId("crossover_high"), 2500.0f);
    splitter.setFrequencies(lowMidFreq, midHighFreq);

    const char* suffixes[3] = { "low", "mid", "high" };
    for (int i = 0; i < 3; ++i) {
        std::string s = suffixes[i];
        bands[i].compressor.setThreshold(loadFloat(paramId("threshold_" + s), -10.0f));
        bands[i].compressor.setRatio(loadFloat(paramId("ratio_" + s), 3.0f));
        bands[i].compressor.setAttack(loadFloat(paramId("attack_" + s), 2.0f));
        bands[i].compressor.setRelease(loadFloat(paramId("release_" + s), 100.0f));
        
        float gain = loadFloat(paramId("gain_" + s), 0.0f);
        bands[i].makeupGain.setGainDecibels(gain);
    }

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& MultibandCompressorModule::getModuleId() const { return moduleId; }
const std::string& MultibandCompressorModule::getName() const { return name; }

} // namespace eeval
