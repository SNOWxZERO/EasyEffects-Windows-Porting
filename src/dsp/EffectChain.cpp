#include "EffectChain.h"

namespace eeval {

EffectChain::EffectChain() {
}

void EffectChain::prepare(const juce::dsp::ProcessSpec& spec) {
    for (auto& mod : modules) {
        mod->prepare(spec);
    }
}

void EffectChain::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    for (auto& mod : modules) {
        mod->process(context);
    }
}

void EffectChain::reset() {
    for (auto& mod : modules) {
        mod->reset();
    }
}

void EffectChain::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    for (auto& mod : modules) {
        mod->updateParameters(apvts);
    }
}

void EffectChain::addModule(std::unique_ptr<EffectModule> module) {
    modules.push_back(std::move(module));
}

void EffectChain::clearModules() {
    modules.clear();
}

} // namespace eeval
