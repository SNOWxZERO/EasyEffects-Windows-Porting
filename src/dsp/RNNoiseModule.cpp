#include "RNNoiseModule.h"
#include <cmath>

namespace eeval {

RNNoiseModule::RNNoiseModule()
    : EffectModule()
{
    // Create states
    stL = rnnoise_create(nullptr);
    stR = rnnoise_create(nullptr);

    procBufferL.resize(rnnoiseFrameSize);
    procBufferR.resize(rnnoiseFrameSize);
    outBufferL.resize(rnnoiseFrameSize);
    outBufferR.resize(rnnoiseFrameSize);
}

RNNoiseModule::~RNNoiseModule()
{
    if (stL) rnnoise_destroy(stL);
    if (stR) rnnoise_destroy(stR);
}

void RNNoiseModule::updateParameters(juce::AudioProcessorValueTreeState& apvts)
{
    enabledParam = apvts.getRawParameterValue(paramId("enabled"));
}

void RNNoiseModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    hostSampleRate = spec.sampleRate;
    currentRatio = (double)rnnoiseRate / hostSampleRate;
    currentInvRatio = hostSampleRate / (double)rnnoiseRate;

    int samplesNeededForFrame = static_cast<int>(hostSampleRate * 0.01);
    
    resampleInL.resize(samplesNeededForFrame);
    resampleInR.resize(samplesNeededForFrame);
    resampleOutL.resize(samplesNeededForFrame);
    resampleOutR.resize(samplesNeededForFrame);

    // Latency is 10ms frame
    latencySamples = static_cast<int>(hostSampleRate * 0.01);

    // Pre-allocate FIFOs for roughly 100ms of audio safety
    int fifoSize = static_cast<int>(hostSampleRate * 0.1);
    fifoBuffer.setSize(2, fifoSize);
    outputFifo.setSize(2, fifoSize);
    
    reset();

    // CRITICAL: Pre-fill output FIFO with silence to match the reported latency
    // This creates a stable delay line and prevents underruns in the first blocks.
    for (int i = 0; i < (int)latencySamples; ++i) {
        outputFifo.setSample(0, outFifoWritePos, 0.0f);
        outputFifo.setSample(1, outFifoWritePos, 0.0f);
        outFifoWritePos = (outFifoWritePos + 1) % outputFifo.getNumSamples();
        samplesInOutFifo++;
    }

    prepareBuffers(spec);
}

void RNNoiseModule::reset() 
{
    fifoReadPos = 0;
    fifoWritePos = 0;
    samplesInFifo = 0;
    
    outFifoReadPos = 0;
    outFifoWritePos = 0;
    samplesInOutFifo = 0;

    fifoBuffer.clear();
    outputFifo.clear();
    
    resamplerL.reset();
    resamplerR.reset();
    resamplerOutL.reset();
    resamplerOutR.reset();
}

void RNNoiseModule::processInternal(juce::AudioBuffer<float>& buffer)
{
    // If the module is not enabled by the user (custom param), we still need to 
    // maintain the latency/FIFO delay to avoid phase jumps when toggling.
    bool processingEnabled = (enabledParam && enabledParam->load() >= 0.5f);
    
    int numSamples = buffer.getNumSamples();
    int fifoSize = fifoBuffer.getNumSamples();
    int outFifoSize = outputFifo.getNumSamples();

    // 1. Push input to FIFO
    for (int s = 0; s < numSamples; ++s) {
        fifoBuffer.setSample(0, fifoWritePos, buffer.getSample(0, s));
        fifoBuffer.setSample(1, fifoWritePos, buffer.getSample(1, s));
        
        fifoWritePos = (fifoWritePos + 1) % fifoSize;
        samplesInFifo++;
    }

    // 2. Process frames as long as we have enough for 10ms of audio
    int samplesNeededForFrame = static_cast<int>(hostSampleRate * 0.01);
    
    while (samplesInFifo >= samplesNeededForFrame) {
        processFrame();
        samplesInFifo -= samplesNeededForFrame;
    }

    // 3. Pull output from Output FIFO
    // Since we pre-filled the output FIFO with latencySamples in prepare(), 
    // we should always have enough to pull numSamples if processing is regular.
    for (int s = 0; s < numSamples; ++s) {
        if (samplesInOutFifo > 0) {
            buffer.setSample(0, s, outputFifo.getSample(0, outFifoReadPos));
            buffer.setSample(1, s, outputFifo.getSample(1, outFifoReadPos));
            
            outFifoReadPos = (outFifoReadPos + 1) % outFifoSize;
            samplesInOutFifo--;
        } else {
            // Safety fallback: should not be reached with proper pre-buffering
            buffer.setSample(0, s, 0.0f);
            buffer.setSample(1, s, 0.0f);
        }
    }
}

void RNNoiseModule::processFrame()
{
    int samplesNeeded = static_cast<int>(hostSampleRate * 0.01);
    
    // a. Pull from input FIFO into scratch resample buffer (no heap alloc)
    for (int i = 0; i < samplesNeeded; ++i) {
        resampleInL[i] = fifoBuffer.getSample(0, fifoReadPos);
        resampleInR[i] = fifoBuffer.getSample(1, fifoReadPos);
        fifoReadPos = (fifoReadPos + 1) % fifoBuffer.getNumSamples();
    }

    // b. Resample host -> 48k
    resamplerL.process(currentRatio, resampleInL.data(), procBufferL.data(), rnnoiseFrameSize);
    resamplerR.process(currentRatio, resampleInR.data(), procBufferR.data(), rnnoiseFrameSize);

    // c. RNNoise Process
    // Check for silence to skip neural network (saves CPU)
    float sumSq = 0;
    for (float v : procBufferL) sumSq += v*v;
    for (float v : procBufferR) sumSq += v*v;

    if (sumSq > 1e-10f && processingEnabled) {
        rnnoise_process_frame(stL, outBufferL.data(), procBufferL.data());
        rnnoise_process_frame(stR, outBufferR.data(), procBufferR.data());
    } else {
        // Pass-through if below noise floor or disabled (latency still applied)
        std::copy(procBufferL.begin(), procBufferL.end(), outBufferL.begin());
        std::copy(procBufferR.begin(), procBufferR.end(), outBufferR.begin());
    }

    // d. Resample 48k -> host
    resamplerOutL.process(currentInvRatio, outBufferL.data(), resampleOutL.data(), samplesNeeded);
    resamplerOutR.process(currentInvRatio, outBufferR.data(), resampleOutR.data(), samplesNeeded);

    // e. Push to output FIFO
    for (int i = 0; i < samplesNeeded; ++i) {
        outputFifo.setSample(0, outFifoWritePos, resampleOutL[i]);
        outputFifo.setSample(1, outFifoWritePos, resampleOutR[i]);
        
        outFifoWritePos = (outFifoWritePos + 1) % outputFifo.getNumSamples();
        samplesInOutFifo++;
    }
}

} // namespace eeval
