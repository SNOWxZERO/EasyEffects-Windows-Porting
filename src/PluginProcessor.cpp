#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/GainModule.h"

EasyEffectsAudioProcessor::EasyEffectsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                     ),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    dspChain.addModule(std::make_unique<eeval::GainModule>());
}

EasyEffectsAudioProcessor::~EasyEffectsAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout EasyEffectsAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Add a simple gain parameter for Phase 1/2 (-24dB to 24dB, default 0dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gain", 1), 
        "Output Gain", 
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f, 1.0f), 
        0.0f));

    return layout;
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
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    dspChain.prepare(spec);
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
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
        
    // Update parameter values
    dspChain.updateParameters(parameters);

    // Execute DSP
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    dspChain.process(context);
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
