#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/GainModule.h"
#include "dsp/CompressorModule.h"

EasyEffectsAudioProcessor::EasyEffectsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                     ),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    dspChain.addModule(std::make_unique<eeval::CompressorModule>()); // Add comp first
    dspChain.addModule(std::make_unique<eeval::GainModule>()); // Then gain
}

EasyEffectsAudioProcessor::~EasyEffectsAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout EasyEffectsAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Gain
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gain", 1), "Output Gain", 
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f, 1.0f), 0.0f));

    // Compressor
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("comp_threshold", 1), "Comp Threshold", juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f, 1.0f), -10.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("comp_ratio", 1), "Comp Ratio", juce::NormalisableRange<float>(1.0f, 100.0f, 0.1f, 1.0f), 3.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("comp_attack", 1), "Comp Attack (ms)", juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 1.0f), 2.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("comp_release", 1), "Comp Release (ms)", juce::NormalisableRange<float>(1.0f, 1000.0f, 1.0f, 1.0f), 100.0f));

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
    // Serialize APVTS to XML array
    auto state = parameters.copyState();
    
    // Future-Proofing: We can add module chain order here as child properties
    // e.g. state.setProperty("CHAIN_ORDER", "Compressor,Gain", nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml != nullptr)
        copyXmlToBinary(*xml, destData);
}

void EasyEffectsAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Try to load XML array into APVTS
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr) {
        if (xmlState->hasTagName(parameters.state.getType())) {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
            // Trigger parameter updates manually for the DSP chain right away
            dspChain.updateParameters(parameters);
        }
    }
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EasyEffectsAudioProcessor();
}
