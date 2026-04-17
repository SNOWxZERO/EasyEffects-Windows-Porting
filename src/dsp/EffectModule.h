#pragma once

#include <JuceHeader.h>
#include <string>

namespace eeval {

class EffectModule {
public:
    virtual ~EffectModule() = default;

    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;
    virtual void process(const juce::dsp::ProcessContextReplacing<float>& context) = 0;
    virtual void reset() = 0;
    
    // Updates internal state from the APVTS if needed
    virtual void updateParameters(juce::AudioProcessorValueTreeState& apvts) = 0;

    void setBypassed(bool bypass) { bypassed = bypass; }
    bool isBypassed() const { return bypassed; }

    virtual const std::string& getName() const = 0;

protected:
    bool bypassed = false;
};

} // namespace eeval
