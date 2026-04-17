#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include "EffectModule.h"

namespace eeval {

class EffectChain {
public:
    EffectChain();
    ~EffectChain() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    void updateParameters(juce::AudioProcessorValueTreeState& apvts);

    void addModule(std::unique_ptr<EffectModule> module);
    void clearModules();

    std::vector<std::string> getModuleNames() const;

private:
    std::vector<std::unique_ptr<EffectModule>> modules;
};

} // namespace eeval
