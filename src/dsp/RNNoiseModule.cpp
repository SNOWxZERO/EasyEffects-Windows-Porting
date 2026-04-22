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
    vadThresholdParam = apvts.getRawParameterValue(paramId("vad_threshold"));
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
    
    // Initialize smoothing (10ms ramp for 10ms frames)
    smoothedVadGainL.reset(hostSampleRate, 0.01);
    smoothedVadGainR.reset(hostSampleRate, 0.01);
    smoothedVadGainL.setCurrentAndTargetValue(1.0f);
    smoothedVadGainR.setCurrentAndTargetValue(1.0f);

    reset();

    // CRITICAL: Pre-fill output FIFO with silence to match the reported latency
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
    juce::ScopedNoDenormals noDenormals;

    // Check enabled status once per block as per user correction
    bool enabled = (enabledParam && enabledParam->load() >= 0.5f);
    
    int numSamples = buffer.getNumSamples();
    int fifoSize = fifoBuffer.getNumSamples();
    int outFifoSize = outputFifo.getNumSamples();

    // 1. Push input to FIFO
    for (int s = 0; s < numSamples; ++s) {
        // Clamp input before pushing to FIFO for stability
        float inL = juce::jlimit(-1.0f, 1.0f, buffer.getSample(0, s));
        float inR = juce::jlimit(-1.0f, 1.0f, buffer.getSample(1, s));

        fifoBuffer.setSample(0, fifoWritePos, inL);
        fifoBuffer.setSample(1, fifoWritePos, inR);
        
        fifoWritePos = (fifoWritePos + 1) % fifoSize;
        samplesInFifo++;
    }

    // 2. Process frames as long as we have enough for 10ms of audio
    int samplesNeededForFrame = static_cast<int>(hostSampleRate * 0.01);
    
    while (samplesInFifo >= samplesNeededForFrame) {
        processFrame(enabled);
        samplesInFifo -= samplesNeededForFrame;
    }

    // 3. Pull output from Output FIFO
    for (int s = 0; s < numSamples; ++s) {
        if (samplesInOutFifo > 0) {
            buffer.setSample(0, s, outputFifo.getSample(0, outFifoReadPos));
            buffer.setSample(1, s, outputFifo.getSample(1, outFifoReadPos));
            
            outFifoReadPos = (outFifoReadPos + 1) % outFifoSize;
            samplesInOutFifo--;
        } else {
            buffer.setSample(0, s, 0.0f);
            buffer.setSample(1, s, 0.0f);
        }
    }
}

void RNNoiseModule::processFrame(bool enabled)
{
    int samplesNeeded = static_cast<int>(hostSampleRate * 0.01);
    float thresh = vadThresholdParam ? vadThresholdParam->load() : 0.5f;
    
    // a. Pull from input FIFO
    for (int i = 0; i < samplesNeeded; ++i) {
        resampleInL[i] = fifoBuffer.getSample(0, fifoReadPos);
        resampleInR[i] = fifoBuffer.getSample(1, fifoReadPos);
        fifoReadPos = (fifoReadPos + 1) % fifoBuffer.getNumSamples();
    }

    // b. Resample host -> 48k
    resamplerL.process(currentRatio, resampleInL.data(), procBufferL.data(), rnnoiseFrameSize);
    resamplerR.process(currentRatio, resampleInR.data(), procBufferR.data(), rnnoiseFrameSize);

    // c. RNNoise Process with Scale and Clamp
    float sumSq = 0;
    for (int i = 0; i < rnnoiseFrameSize; ++i) {
        // Correct Input Scaling (32768.0f)
        procBufferL[i] *= 32768.0f;
        procBufferR[i] *= 32768.0f;
        sumSq += procBufferL[i] * procBufferL[i] + procBufferR[i] * procBufferR[i];
    }

    float vadProbL = 1.0f;
    float vadProbR = 1.0f;

    // CPU Optimization: skip if silence or disabled
    if (sumSq > 0.1f && enabled) { // Threshold adjusted for 16-bit range
        vadProbL = rnnoise_process_frame(stL, outBufferL.data(), procBufferL.data());
        vadProbR = rnnoise_process_frame(stR, outBufferR.data(), procBufferR.data());
    } else {
        std::copy(procBufferL.begin(), procBufferL.end(), outBufferL.begin());
        std::copy(procBufferR.begin(), procBufferR.end(), outBufferR.begin());
    }

    // d. Post-Process (Scale Back, Clamp, Soft Gate)
    // Update smoothing targets based on VAD
    smoothedVadGainL.setTargetValue(vadProbL < thresh ? softGateFloor : 1.0f);
    smoothedVadGainR.setTargetValue(vadProbR < thresh ? softGateFloor : 1.0f);

    for (int i = 0; i < rnnoiseFrameSize; ++i) {
        // Output Scaling and Smoothing
        outBufferL[i] = juce::jlimit(-1.0f, 1.0f, (outBufferL[i] / 32768.0f)) * smoothedVadGainL.getNextValue();
        outBufferR[i] = juce::jlimit(-1.0f, 1.0f, (outBufferR[i] / 32768.0f)) * smoothedVadGainR.getNextValue();
    }

    // e. Resample 48k -> host
    resamplerOutL.process(currentInvRatio, outBufferL.data(), resampleOutL.data(), samplesNeeded);
    resamplerOutR.process(currentInvRatio, outBufferR.data(), resampleOutR.data(), samplesNeeded);

    // f. Push to output FIFO
    for (int i = 0; i < samplesNeeded; ++i) {
        outputFifo.setSample(0, outFifoWritePos, resampleOutL[i]);
        outputFifo.setSample(1, outFifoWritePos, resampleOutR[i]);
        
        outFifoWritePos = (outFifoWritePos + 1) % outputFifo.getNumSamples();
        samplesInOutFifo++;
    }
}

} // namespace eeval
