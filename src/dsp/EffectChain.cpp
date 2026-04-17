#include "EffectChain.h"

namespace eeval {

EffectChain::EffectChain() {
}

void EffectChain::prepare(const juce::dsp::ProcessSpec& spec) {
    for (auto& mod : modules) {
        mod->prepare(spec);
    }
}

void EffectChain::process(juce::AudioBuffer<float>& buffer) {
    for (auto& mod : modules) {
        mod->process(buffer);
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

std::vector<std::string> EffectChain::getModuleNames() const {
    std::vector<std::string> names;
    for (const auto& mod : modules) {
        names.push_back(mod->getName());
    }
    return names;
}

std::vector<std::string> EffectChain::getModuleIds() const {
    std::vector<std::string> ids;
    for (const auto& mod : modules) {
        ids.push_back(mod->getModuleId());
    }
    return ids;
}

} // namespace eeval
