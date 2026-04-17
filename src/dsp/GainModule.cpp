#include "GainModule.h"

namespace eeval {

GainModule::GainModule() {
}

void GainModule::prepare(const juce::dsp::ProcessSpec& spec) {
    gainNode.prepare(spec);
    gainNode.setRampDurationSeconds(0.05); // 50ms ramp to avoid clicks
}

void GainModule::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    if (isBypassed()) return;
    gainNode.process(context);
}

void GainModule::reset() {
    gainNode.reset();
}

void GainModule::updateParameters(juce::AudioProcessorValueTreeState& apvts) {
    auto gainParam = apvts.getRawParameterValue("gain");
    if (gainParam != nullptr) {
        gainNode.setGainDecibels(gainParam->load());
    }
}

const std::string& GainModule::getName() const {
    return name;
}

} // namespace eeval
