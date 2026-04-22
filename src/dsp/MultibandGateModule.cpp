#include "MultibandGateModule.h"

namespace eeval {

MultibandGateModule::MultibandGateModule() {}

void MultibandGateModule::prepare(const juce::dsp::ProcessSpec& spec) {
    splitter.prepare(spec);
    for (int i = 0; i < 3; ++i) {
        bands[i].prepare(spec);
        bandBuffers[i].setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    }
    prepareBuffers(spec);
}

void MultibandGateModule::reset() {
    splitter.reset();
    for (int i = 0; i < 3; ++i) {
        bands[i].reset();
    }
}

void MultibandGateModule::processInternal(juce::AudioBuffer<float>& buffer) {
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    // 1. Split
    for (int ch = 0; ch < numChannels; ++ch) {
        auto* input = buffer.getReadPointer(ch);
        auto* b0 = bandBuffers[0].getWritePointer(ch);
        auto* b1 = bandBuffers[1].getWritePointer(ch);
        auto* b2 = bandBuffers[2].getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i) {
            splitter.processSample(ch, input[i], b0[i], b1[i], b2[i]);
        }
    }

    // 2. Process
    for (int i = 0; i < 3; ++i) {
        juce::dsp::AudioBlock<float> block(bandBuffers[i]);
        auto subBlock = block.getSubBlock(0, (size_t)numSamples);
        juce::dsp::ProcessContextReplacing<float> context(subBlock);
        bands[i].gate.process(context);
    }

    // 3. Recombine
    buffer.clear();
    for (int i = 0; i < 3; ++i) {
        for (int ch = 0; ch < numChannels; ++ch) {
            buffer.addFrom(ch, 0, bandBuffers[i], ch, 0, numSamples);
        }
    }
}

void MultibandGateModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto loadFloat = [&](const std::string& id, float defaultVal) {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return defaultVal;
    };

    lowMidFreq = loadFloat(paramId("crossover_low"), 200.0f);
    midHighFreq = loadFloat(paramId("crossover_high"), 2500.0f);
    splitter.setFrequencies(lowMidFreq, midHighFreq);

    const char* suffixes[3] = { "low", "mid", "high" };
    for (int i = 0; i < 3; ++i) {
        std::string s = suffixes[i];
        bands[i].gate.setThreshold(loadFloat(paramId("threshold_" + s), -40.0f));
        bands[i].gate.setRatio(loadFloat(paramId("ratio_" + s), 10.0f));
        bands[i].gate.setAttack(loadFloat(paramId("attack_" + s), 1.0f));
        bands[i].gate.setRelease(loadFloat(paramId("release_" + s), 100.0f));
    }

    setDryWetMix(loadFloat(slotParamId("mix"), 100.0f) / 100.0f);
    setBypassed(loadFloat(slotParamId("bypass"), 0.0f) > 0.5f);
}

const std::string& MultibandGateModule::getModuleId() const { return moduleId; }
const std::string& MultibandGateModule::getName() const { return name; }

} // namespace eeval
