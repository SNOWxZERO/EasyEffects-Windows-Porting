#pragma once
#include "EffectModule.h"
#include "BandSplitter.h"

namespace eeval {

class MultibandCompressorModule : public EffectModule {
public:
    MultibandCompressorModule();
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void reset() override;

    const std::string& getModuleId() const override;
    const std::string& getName() const override;

private:
    std::string moduleId = "multiband_compressor";
    std::string name = "Multiband Compressor";

    BandSplitter splitter;
    
    struct BandProcessor {
        juce::dsp::Compressor<float> compressor;
        juce::dsp::Gain<float> makeupGain;
        float gainDb = 0.0f;

        void prepare(const juce::dsp::ProcessSpec& spec) {
            compressor.prepare(spec);
            makeupGain.prepare(spec);
            makeupGain.setRampDurationSeconds(0.02);
        }

        void reset() {
            compressor.reset();
            makeupGain.reset();
        }

        float processSample(int channel, float sample) {
            // Compressor only works on blocks usually, but we can do it sample by sample 
            // if we use the right API. However, juce::dsp::Compressor operates on blocks.
            // For sample-by-sample, we need to manually apply.
            // Actually, I'll process in blocks for efficiency after splitting.
            return sample; // Placeholder
        }
    };

    BandProcessor bands[3];
    juce::AudioBuffer<float> bandBuffers[3];
    float lowMidFreq = 200.0f;
    float midHighFreq = 2500.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultibandCompressorModule)
};

} // namespace eeval
