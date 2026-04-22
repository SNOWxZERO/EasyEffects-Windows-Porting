#include "PitchShiftModule.h"
#include <cmath>

namespace eeval {

PitchShiftModule::PitchShiftModule()
    : EffectModule()
{
    st = std::make_unique<soundtouch::SoundTouch>();
}

void PitchShiftModule::updateParameters(juce::AudioProcessorValueTreeState& apvts)
{
    pitchParam = apvts.getRawParameterValue(paramId("semitones"));
}

void PitchShiftModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    st->setSampleRate(static_cast<uint>(spec.sampleRate));
    st->setChannels(spec.numChannels);
    
    st->setSetting(SETTING_USE_QUICKSEEK, 0); 
    st->setSetting(SETTING_USE_AA_FILTER, 1);
    
    interleavedBuffer.resize(spec.maximumBlockSize * spec.numChannels);
    outputInterleaved.resize(spec.maximumBlockSize * spec.numChannels * 2);

    reset();
    
    latencySamples = st->getSetting(SETTING_INITIAL_LATENCY);
}

void PitchShiftModule::reset()
{
    st->clear();
    lastPitch = 0.0f;
}

void PitchShiftModule::processInternal(juce::AudioBuffer<float>& buffer)
{
    if (pitchParam) {
        float p = pitchParam->load();
        if (std::abs(p - lastPitch) > 0.01f) {
            st->setPitchSemiTones(p);
            lastPitch = p;
        }
    }

    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    for (int s = 0; s < numSamples; ++s) {
        for (int c = 0; c < numChannels; ++c) {
            interleavedBuffer[s * numChannels + c] = buffer.getSample(c, s);
        }
    }

    st->putSamples(interleavedBuffer.data(), static_cast<uint>(numSamples));

    uint samplesReceived = st->receiveSamples(outputInterleaved.data(), static_cast<uint>(numSamples));

    if (samplesReceived >= static_cast<uint>(numSamples)) {
        for (int s = 0; s < numSamples; ++s) {
            for (int c = 0; c < numChannels; ++c) {
                buffer.setSample(c, s, outputInterleaved[s * numChannels + c]);
            }
        }
    } else {
        for (uint s = 0; s < samplesReceived; ++s) {
            for (int c = 0; c < numChannels; ++c) {
                buffer.setSample(c, s, outputInterleaved[s * numChannels + c]);
            }
        }
        for (int s = static_cast<int>(samplesReceived); s < numSamples; ++s) {
            for (int c = 0; c < numChannels; ++c) {
                buffer.setSample(c, s, 0.0f);
            }
        }
    }
}

} // namespace eeval
