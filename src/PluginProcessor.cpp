#include "PluginProcessor.h"
#include "PluginEditor.h"

EasyEffectsAudioProcessor::EasyEffectsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                     )
#endif
{
}

EasyEffectsAudioProcessor::~EasyEffectsAudioProcessor()
{
}

const juce::String EasyEffectsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EasyEffectsAudioProcessor::acceptsMidi() const
{
    return false;
}

bool EasyEffectsAudioProcessor::producesMidi() const
{
    return false;
}

bool EasyEffectsAudioProcessor::isMidiEffect() const
{
    return false;
}

double EasyEffectsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EasyEffectsAudioProcessor::getNumPrograms()
{
    return 1;
}

int EasyEffectsAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EasyEffectsAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String EasyEffectsAudioProcessor::getProgramName(int index)
{
    return {};
}

void EasyEffectsAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void EasyEffectsAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Phase 1 / Phase 2: We will setup DSP processing here
}

void EasyEffectsAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool EasyEffectsAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void EasyEffectsAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
        
    // In Phase 1, this currently acts as a clean audio passthrough!
}

bool EasyEffectsAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EasyEffectsAudioProcessor::createEditor()
{
    return new EasyEffectsAudioProcessorEditor(*this);
}

void EasyEffectsAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Phase 5 Preset Saving logic goes here
}

void EasyEffectsAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Phase 5 Preset Loading logic goes here
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EasyEffectsAudioProcessor();
}
