#pragma once
#include "EffectModule.h"

namespace eeval {

class ConvolverModule : public EffectModule {
public:
    ConvolverModule();
    ~ConvolverModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;
    const std::string& getModuleId() const override;
    const std::string& getName() const override;
    
    juce::var getState() override;
    void setState(const juce::var& state) override;

    // Call this to load an Impulse Response File
    void loadImpulseResponse(const juce::File& file);
    
    const std::string& getLoadedIRName() const { return irName; }

private:
    juce::dsp::Convolution convolverNode;
    std::atomic<bool> hasIRLoaded { false };
    
    std::string irPath;
    std::string irName;

    std::string moduleId = "convolver";
    std::string name = "Convolver";
};

} // namespace eeval
