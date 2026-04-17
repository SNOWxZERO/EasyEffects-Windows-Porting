#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/GainModule.h"
#include "dsp/CompressorModule.h"
#include "dsp/GateModule.h"
#include "dsp/EqualizerModule.h"
#include "dsp/LimiterModule.h"
#include "dsp/FilterModule.h"
#include "dsp/DelayModule.h"
#include "dsp/ReverbModule.h"
#include "dsp/ExciterModule.h"
#include "dsp/BassEnhancerModule.h"
#include "dsp/DeesserModule.h"
#include "dsp/ConvolverModule.h"
#include "dsp/LevelMeterModule.h"

EasyEffectsAudioProcessor::EasyEffectsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                     ),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    // Fixed chain order: Gate → EQ → Deesser → Exciter → BassEnhancer → Compressor → Filter → Convolver → Delay → Reverb → Limiter → LevelMeter → Gain
    dspChain.addModule(std::make_unique<eeval::GateModule>());
    dspChain.addModule(std::make_unique<eeval::EqualizerModule>());
    dspChain.addModule(std::make_unique<eeval::DeesserModule>());
    dspChain.addModule(std::make_unique<eeval::ExciterModule>());
    dspChain.addModule(std::make_unique<eeval::BassEnhancerModule>());
    dspChain.addModule(std::make_unique<eeval::CompressorModule>());
    dspChain.addModule(std::make_unique<eeval::FilterModule>());
    dspChain.addModule(std::make_unique<eeval::ConvolverModule>());
    dspChain.addModule(std::make_unique<eeval::DelayModule>());
    dspChain.addModule(std::make_unique<eeval::ReverbModule>());
    dspChain.addModule(std::make_unique<eeval::LimiterModule>());
    
    auto meterNode = std::make_unique<eeval::LevelMeterModule>();
    levelMeterPtr = meterNode.get();
    dspChain.addModule(std::move(meterNode));
    
    dspChain.addModule(std::make_unique<eeval::GainModule>());
}

EasyEffectsAudioProcessor::~EasyEffectsAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout EasyEffectsAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Helper lambda for adding a standard float param
    auto addFloat = [&](const std::string& id, const juce::String& name,
                        float min, float max, float step, float defaultVal) {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id, 1), name,
            juce::NormalisableRange<float>(min, max, step), defaultVal));
    };

    auto addBool = [&](const std::string& id, const juce::String& name, bool defaultVal) {
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(id, 1), name, defaultVal));
    };

    auto addChoice = [&](const std::string& id, const juce::String& name, const juce::StringArray& choices, int defaultIndex) {
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(id, 1), name, choices, defaultIndex));
    };

    // === Gate ===
    addFloat("gate.threshold", "Gate Threshold", -60.0f, 0.0f, 0.1f, -40.0f);
    addFloat("gate.ratio", "Gate Ratio", 1.0f, 20.0f, 0.1f, 10.0f);
    addFloat("gate.attack", "Gate Attack (ms)", 0.1f, 100.0f, 0.1f, 1.0f);
    addFloat("gate.release", "Gate Release (ms)", 1.0f, 1000.0f, 1.0f, 100.0f);
    addFloat("gate.mix", "Gate Mix", 0.0f, 100.0f, 1.0f, 100.0f);
    addBool("gate.bypass", "Gate Bypass", false);

    // === Equalizer ===
    for (int i = 0; i < eeval::EqualizerModule::NUM_BANDS; ++i) {
        std::string p = "eq.band" + std::to_string(i);
        addFloat(p + ".gain", juce::String("EQ Band ") + juce::String(i + 1) + " Gain", -24.0f, 24.0f, 0.1f, 0.0f);
        
        // Defaults: 100Hz (Low Shelf), 1000Hz (Mid1), 4000Hz (Mid2), 10000Hz (High Shelf)
        float freqDef = (i == 0) ? 100.0f : (i == 1) ? 1000.0f : (i == 2) ? 4000.0f : 10000.0f;
        addFloat(p + ".freq", juce::String("EQ Band ") + juce::String(i + 1) + " Freq", 20.0f, 20000.0f, 1.0f, freqDef);
        addFloat(p + ".q", juce::String("EQ Band ") + juce::String(i + 1) + " Q", 0.1f, 10.0f, 0.05f, 0.707f);
    }
    addFloat("eq.mix", "EQ Mix", 0.0f, 100.0f, 1.0f, 100.0f);
    addBool("eq.bypass", "EQ Bypass", false);

    // === Deesser ===
    addFloat("deesser.threshold", "Deesser Threshold", -60.0f, 0.0f, 0.1f, -20.0f);
    addFloat("deesser.ratio", "Deesser Ratio", 1.0f, 100.0f, 0.1f, 5.0f);
    addFloat("deesser.frequency", "Deesser Split Freq", 2000.0f, 20000.0f, 10.0f, 6000.0f);
    addFloat("deesser.mix", "Deesser Mix", 0.0f, 100.0f, 1.0f, 100.0f);
    addBool("deesser.bypass", "Deesser Bypass", false);

    // === Exciter ===
    addFloat("exciter.amount", "Exciter Amount", 0.0f, 1.0f, 0.01f, 0.2f);
    addFloat("exciter.harmonics", "Exciter Harmonics", 0.0f, 1.0f, 0.01f, 0.5f);
    addFloat("exciter.cutoff", "Exciter Cutoff", 2000.0f, 20000.0f, 10.0f, 4000.0f);
    addFloat("exciter.mix", "Exciter Mix", 0.0f, 100.0f, 1.0f, 100.0f);
    addBool("exciter.bypass", "Exciter Bypass", false);

    // === Bass Enhancer ===
    addFloat("bassenhancer.amount", "Bass Enhancer Amount", 0.0f, 1.0f, 0.01f, 0.2f);
    addFloat("bassenhancer.harmonics", "Bass Enhancer Harmonics", 0.0f, 1.0f, 0.01f, 0.5f);
    addFloat("bassenhancer.cutoff", "Bass Enhancer Cutoff", 20.0f, 1000.0f, 10.0f, 150.0f);
    addFloat("bassenhancer.mix", "Bass Enhancer Mix", 0.0f, 100.0f, 1.0f, 100.0f);
    addBool("bassenhancer.bypass", "Bass Enhancer Bypass", false);

    // === Compressor ===
    addFloat("compressor.threshold", "Compressor Threshold", -60.0f, 0.0f, 0.1f, -10.0f);
    addFloat("compressor.ratio", "Compressor Ratio", 1.0f, 100.0f, 0.1f, 3.0f);
    addFloat("compressor.attack", "Compressor Attack (ms)", 0.1f, 100.0f, 0.1f, 2.0f);
    addFloat("compressor.release", "Compressor Release (ms)", 1.0f, 1000.0f, 1.0f, 100.0f);
    addFloat("compressor.mix", "Compressor Mix", 0.0f, 100.0f, 1.0f, 100.0f);
    addBool("compressor.bypass", "Compressor Bypass", false);
    
    // === Limiter ===
    addFloat("limiter.threshold", "Limiter Threshold", -60.0f, 0.0f, 0.1f, -1.0f);
    addFloat("limiter.release", "Limiter Release (ms)", 1.0f, 1000.0f, 1.0f, 100.0f);
    addFloat("limiter.mix", "Limiter Mix", 0.0f, 100.0f, 1.0f, 100.0f);
    addBool("limiter.bypass", "Limiter Bypass", false);

    // === Filter ===
    addChoice("filter.type", "Filter Type", juce::StringArray{"Highpass", "Lowpass"}, 0);
    addFloat("filter.cutoff", "Filter Cutoff (Hz)", 20.0f, 20000.0f, 1.0f, 100.0f);
    addFloat("filter.resonance", "Filter Resonance", 0.1f, 10.0f, 0.05f, 0.707f);
    addFloat("filter.mix", "Filter Mix", 0.0f, 100.0f, 1.0f, 100.0f);
    addBool("filter.bypass", "Filter Bypass", false);

    // === Convolver ===
    addFloat("convolver.mix", "Convolver Mix", 0.0f, 100.0f, 1.0f, 100.0f);
    addBool("convolver.bypass", "Convolver Bypass", false);

    // === Delay ===
    addFloat("delay.time_ms", "Delay Time (ms)", 0.0f, 2000.0f, 1.0f, 200.0f);
    addFloat("delay.feedback", "Delay Feedback %", 0.0f, 100.0f, 1.0f, 30.0f);
    addFloat("delay.delay_mix", "Internal Delay Mix %", 0.0f, 100.0f, 1.0f, 50.0f);
    addFloat("delay.mix", "Delay Module Mix", 0.0f, 100.0f, 1.0f, 100.0f);
    addBool("delay.bypass", "Delay Bypass", false);

    // === Reverb ===
    addFloat("reverb.room_size", "Reverb Room Size", 0.0f, 1.0f, 0.01f, 0.5f);
    addFloat("reverb.damping", "Reverb Damping", 0.0f, 1.0f, 0.01f, 0.5f);
    addFloat("reverb.wet", "Reverb Wet Level", 0.0f, 1.0f, 0.01f, 0.33f);
    addFloat("reverb.dry", "Reverb Dry Level", 0.0f, 1.0f, 0.01f, 0.4f);
    addFloat("reverb.width", "Reverb Width", 0.0f, 1.0f, 0.01f, 1.0f);
    addFloat("reverb.mix", "Reverb Mix", 0.0f, 100.0f, 1.0f, 100.0f);
    addBool("reverb.bypass", "Reverb Bypass", false);

    // === Gain ===
    addFloat("gain.level", "Output Gain", -24.0f, 24.0f, 0.1f, 0.0f);
    addFloat("gain.mix", "Gain Mix", 0.0f, 100.0f, 1.0f, 100.0f);
    addBool("gain.bypass", "Gain Bypass", false);

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

    // Execute DSP (buffer-based, no allocations)
    dspChain.process(buffer);
}

bool EasyEffectsAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

eeval::LevelMeterModule* EasyEffectsAudioProcessor::getLevelMeter()
{
    return levelMeterPtr;
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
