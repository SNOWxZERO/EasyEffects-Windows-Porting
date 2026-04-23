#include "EffectModule.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <rnnoise.h>
#include <memory>
#include <vector>

namespace eeval {

/**
 * RNNoiseModule - Real-time noise suppression using Recurrent Neural Networks.
 * 
 * Optimized for Windows with a robust FIFO resampling pipeline and 
 * pre-buffered latency compensation.
 */
class RNNoiseModule : public EffectModule {
public:
    RNNoiseModule();
    ~RNNoiseModule() override;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void processInternal(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    void updateParameters(juce::AudioProcessorValueTreeState& apvts) override;

    double getLatencySamples() const override { return latencySamples; }
    float getVADProbability() const override { return latestVad.load(); }

    const std::string& getModuleId() const override { return moduleId; }
    const std::string& getName() const override { return name; }

private:
    std::string moduleId = "rnnoise";
    std::string name = "Noise Reduction";

    // RNNoise States (Stereo)
    DenoiseState* stL = nullptr;
    DenoiseState* stR = nullptr;

    const int rnnoiseRate = 48000;
    const int rnnoiseFrameSize = 480;

    double hostSampleRate = 44100.0;
    double currentRatio = 1.0;
    double currentInvRatio = 1.0;
    
    // Resampling
    juce::LagrangeInterpolator resamplerL;
    juce::LagrangeInterpolator resamplerR;
    juce::LagrangeInterpolator resamplerOutL;
    juce::LagrangeInterpolator resamplerOutR;

    // Pre-allocated buffers for 48kHz processing (Real-time safe)
    std::vector<float> procBufferL;
    std::vector<float> procBufferR;
    std::vector<float> outBufferL;
    std::vector<float> outBufferR;
    
    // Pre-allocated scratch buffers for resampling
    std::vector<float> resampleInL;
    std::vector<float> resampleInR;
    std::vector<float> resampleOutL;
    std::vector<float> resampleOutR;

    // FIFOs to handle rate transition and frame alignment
    juce::AudioBuffer<float> fifoBuffer;
    int fifoReadPos = 0;
    int fifoWritePos = 0;
    int samplesInFifo = 0;

    juce::AudioBuffer<float> outputFifo;
    int outFifoReadPos = 0;
    int outFifoWritePos = 0;
    int samplesInOutFifo = 0;

    // Parameter
    std::atomic<float>* enabledParam = nullptr;
    std::atomic<float>* vadThresholdParam = nullptr;
    double latencySamples = 0.0;
    std::atomic<float> latestVad { 0.0f };

    // Smoothing for VAD-based gating
    juce::LinearSmoothedValue<float> smoothedVadGainL;
    juce::LinearSmoothedValue<float> smoothedVadGainR;
    
    // Minimum gain when no VAD is detected (Soft gate floor)
    const float softGateFloor = 0.001f; // -60dB

    void processFrame(bool enabled);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RNNoiseModule)
};

} // namespace eeval
