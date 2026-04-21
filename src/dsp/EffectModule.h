#pragma once

#include <JuceHeader.h>
#include <string>

namespace eeval {

class EffectModule {
public:
    virtual ~EffectModule() = default;

    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;
    virtual void reset() = 0;
    
    // Subclasses implement their core DSP here (called only when not bypassed)
    virtual void processInternal(juce::AudioBuffer<float>& buffer) = 0;

    // Updates internal state from the APVTS
    virtual void updateParameters(juce::AudioProcessorValueTreeState& apvts) = 0;

    // Returns the module ID used for parameter naming: "<moduleId>.<param>"
    virtual const std::string& getModuleId() const = 0;

    // Returns a human-readable display name
    virtual const std::string& getName() const = 0;

    // Returns the latency in samples (if any)
    virtual double getLatencySamples() const { return 0.0; }

    // Main process entry point — handles bypass and dry/wet mix
    void process(juce::AudioBuffer<float>& buffer)
    {
        if (bypassed) return;

        if (dryWetMix >= 1.0f) {
            // Fully wet — no need to copy
            processInternal(buffer);
        } else if (dryWetMix <= 0.0f) {
            // Fully dry — skip processing entirely
            return;
        } else {
            // Blend: copy dry, process wet, mix
            dryBuffer.makeCopyOf(buffer, true);
            processInternal(buffer);

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                auto* wet = buffer.getWritePointer(ch);
                const auto* dry = dryBuffer.getReadPointer(ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i) {
                    wet[i] = dry[i] * (1.0f - dryWetMix) + wet[i] * dryWetMix;
                }
            }
        }
    }

    void setBypassed(bool bypass) { bypassed = bypass; }
    bool isBypassed() const { return bypassed; }

    void setDryWetMix(float mix) { dryWetMix = juce::jlimit(0.0f, 1.0f, mix); }
    float getDryWetMix() const { return dryWetMix; }

    // Slot-level identifier (e.g., "slot3")
    void setSlotPrefix(const std::string& prefix) { slotPrefix = prefix; }
    const std::string& getSlotPrefix() const { return slotPrefix; }

    // Effect-level parameter prefix (e.g., "slot3.compressor")
    void setParamPrefix(const std::string& prefix) { slotParamPrefix = prefix; }
    const std::string& getParamPrefix() const { return slotParamPrefix; }

    // Helper: builds a parameter ID string for effect-specific parameters
    // "slot3.compressor.threshold"
    std::string paramId(const std::string& paramName) const {
        if (!slotParamPrefix.empty())
            return slotParamPrefix + "." + paramName;
        return getModuleId() + "." + paramName;
    }

    // Helper: builds a parameter ID string for common slot parameters
    // "slot3.mix", "slot3.bypass"
    std::string slotParamId(const std::string& paramName) const {
        if (!slotPrefix.empty())
            return slotPrefix + "." + paramName;
        return paramName;
    }

protected:
    bool bypassed = false;
    float dryWetMix = 1.0f; // 1.0 = fully wet (effect applied)
    std::string slotPrefix;      // e.g., "slot3"
    std::string slotParamPrefix; // e.g., "slot3.compressor"

    // Pre-allocated in prepare() for dry/wet blending
    juce::AudioBuffer<float> dryBuffer;

    // Call in subclass prepare() to set up the dry buffer
    void prepareBuffers(const juce::dsp::ProcessSpec& spec) {
        dryBuffer.setSize((int)spec.numChannels, (int)spec.maximumBlockSize);
    }
};

} // namespace eeval
