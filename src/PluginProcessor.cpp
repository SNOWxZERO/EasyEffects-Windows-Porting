#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/EffectRegistry.h"
#include "dsp/LevelMeterModule.h"
#include <map>

EasyEffectsAudioProcessor::EasyEffectsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                     ),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    // Start with a default chain: Gate → Compressor → Limiter → Gain
    // Users can add/remove/reorder from the UI
    addEffect("gate");
    addEffect("compressor");
    addEffect("limiter");
    addEffect("gain");
}

EasyEffectsAudioProcessor::~EasyEffectsAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout EasyEffectsAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Register parameters for all 16 slots using the EffectRegistry
    for (int i = 0; i < eeval::EffectRegistry::MAX_SLOTS; ++i) {
        eeval::EffectRegistry::registerSlotParameters(layout, i);
    }

    return layout;
}

// --- Dynamic Chain Management ---

int EasyEffectsAudioProcessor::addEffect(const std::string& typeId) {
    // Find the first empty slot
    for (int i = 0; i < eeval::EffectChain::MAX_SLOTS; ++i) {
        if (dspChain.getSlotTypeId(i) == "none") {
            std::string prefix = eeval::EffectRegistry::slotPrefix(i);
            auto module = eeval::EffectRegistry::createModule(typeId, prefix);
            if (module) {
                dspChain.setSlotTypeId(i, typeId);
                dspChain.setSlotModule(i, std::move(module));

                // Set the slot type choice parameter
                auto* typeParam = parameters.getParameter(prefix + ".type");
                if (typeParam) {
                    auto choices = eeval::EffectRegistry::getTypeChoices();
                    int choiceIndex = choices.indexOf(typeId);
                    if (choiceIndex >= 0)
                        typeParam->setValueNotifyingHost(typeParam->convertTo0to1((float)choiceIndex));
                }

                return i;
            }
            return -1;
        }
    }
    return -1; // No empty slots
}

void EasyEffectsAudioProcessor::removeEffect(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= eeval::EffectChain::MAX_SLOTS) return;

    dspChain.clearSlot(slotIndex);

    // Reset the slot type parameter to "none"
    std::string prefix = eeval::EffectRegistry::slotPrefix(slotIndex);
    auto* typeParam = parameters.getParameter(prefix + ".type");
    if (typeParam)
        typeParam->setValueNotifyingHost(0.0f); // Index 0 = "none"

    // Compact: shift all slots above down to fill the gap
    for (int i = slotIndex; i < eeval::EffectChain::MAX_SLOTS - 1; ++i) {
        if (dspChain.getSlotTypeId(i + 1) != "none") {
            // Move slot i+1 into slot i
            std::string fromTypeId = dspChain.getSlotTypeId(i + 1);
            std::string newPrefix = eeval::EffectRegistry::slotPrefix(i);
            auto module = eeval::EffectRegistry::createModule(fromTypeId, newPrefix);
            if (module) {
                // Copy parameter values from old slot to new slot
                const auto* desc = eeval::EffectRegistry::findType(fromTypeId);
                if (desc) {
                    std::string oldPrefix = eeval::EffectRegistry::slotPrefix(i + 1) + "." + fromTypeId;
                    std::string newFullPrefix = newPrefix + "." + fromTypeId;
                    for (const auto& p : desc->params) {
                        auto* src = parameters.getRawParameterValue(oldPrefix + "." + p.suffix);
                        auto* dst = parameters.getParameter(newFullPrefix + "." + p.suffix);
                        if (src && dst)
                            dst->setValueNotifyingHost(dst->convertTo0to1(src->load()));
                    }
                    // Copy bypass and mix
                    auto* srcBypass = parameters.getRawParameterValue(eeval::EffectRegistry::slotPrefix(i + 1) + ".bypass");
                    auto* dstBypass = parameters.getParameter(newPrefix + ".bypass");
                    if (srcBypass && dstBypass)
                        dstBypass->setValueNotifyingHost(srcBypass->load() > 0.5f ? 1.0f : 0.0f);

                    auto* srcMix = parameters.getRawParameterValue(eeval::EffectRegistry::slotPrefix(i + 1) + ".mix");
                    auto* dstMix = parameters.getParameter(newPrefix + ".mix");
                    if (srcMix && dstMix)
                        dstMix->setValueNotifyingHost(dstMix->convertTo0to1(srcMix->load()));
                }

                dspChain.setSlotTypeId(i, fromTypeId);
                dspChain.setSlotModule(i, std::move(module));

                auto* tp = parameters.getParameter(newPrefix + ".type");
                if (tp) {
                    auto choices = eeval::EffectRegistry::getTypeChoices();
                    int ci = choices.indexOf(fromTypeId);
                    if (ci >= 0) tp->setValueNotifyingHost(tp->convertTo0to1((float)ci));
                }
            }

            // Clear the old slot
            dspChain.clearSlot(i + 1);
            auto* oldTp = parameters.getParameter(eeval::EffectRegistry::slotPrefix(i + 1) + ".type");
            if (oldTp) oldTp->setValueNotifyingHost(0.0f);
        } else {
            break; // No more effects after this
        }
    }
}

void EasyEffectsAudioProcessor::moveEffect(int slotIndex, int direction) {
    int targetIndex = slotIndex + direction;
    if (slotIndex < 0 || slotIndex >= eeval::EffectChain::MAX_SLOTS) return;
    if (targetIndex < 0 || targetIndex >= eeval::EffectChain::MAX_SLOTS) return;
    if (dspChain.getSlotTypeId(slotIndex) == "none") return;
    if (direction == 1 && dspChain.getSlotTypeId(targetIndex) == "none") return;

    std::string typeA = dspChain.getSlotTypeId(slotIndex);
    std::string typeB = dspChain.getSlotTypeId(targetIndex);

    std::string prefixA = eeval::EffectRegistry::slotPrefix(slotIndex);   // e.g. "slot0"
    std::string prefixB = eeval::EffectRegistry::slotPrefix(targetIndex); // e.g. "slot1"

    // Step 1: Save ALL parameter values from both slots into temp maps
    std::map<std::string, float> valsA, valsB;

    auto saveSlotParams = [&](const std::string& prefix, const std::string& tid, std::map<std::string, float>& dest) {
        // Save common params (bypass, mix)
        auto* bp = parameters.getRawParameterValue(prefix + ".bypass");
        if (bp) dest["bypass"] = bp->load();
        auto* mx = parameters.getRawParameterValue(prefix + ".mix");
        if (mx) dest["mix"] = mx->load();

        // Save type-specific params
        const auto* desc = eeval::EffectRegistry::findType(tid);
        if (desc) {
            for (const auto& p : desc->params) {
                auto* v = parameters.getRawParameterValue(prefix + "." + tid + "." + p.suffix);
                if (v) dest[tid + "." + p.suffix] = v->load();
            }
            for (const auto& c : desc->choices) {
                auto* v = parameters.getRawParameterValue(prefix + "." + tid + "." + c.suffix);
                if (v) dest[tid + "." + c.suffix] = v->load();
            }
        }
    };

    saveSlotParams(prefixA, typeA, valsA);
    if (typeB != "none")
        saveSlotParams(prefixB, typeB, valsB);

    // Step 2: Write A's values into B's slot parameters, and vice versa
    auto writeSlotParams = [&](const std::string& prefix, const std::string& tid, const std::map<std::string, float>& src) {
        auto setParam = [&](const std::string& id, float val) {
            auto* param = parameters.getParameter(id);
            if (param) param->setValueNotifyingHost(param->convertTo0to1(val));
        };

        if (src.count("bypass")) setParam(prefix + ".bypass", src.at("bypass"));
        if (src.count("mix")) setParam(prefix + ".mix", src.at("mix"));

        const auto* desc = eeval::EffectRegistry::findType(tid);
        if (desc) {
            for (const auto& p : desc->params) {
                std::string key = tid + "." + p.suffix;
                if (src.count(key))
                    setParam(prefix + "." + key, src.at(key));
            }
            for (const auto& c : desc->choices) {
                std::string key = tid + "." + c.suffix;
                if (src.count(key))
                    setParam(prefix + "." + key, src.at(key));
            }
        }
    };

    // Write A's saved values into slot B's parameter space
    writeSlotParams(prefixB, typeA, valsA);
    // Write B's saved values into slot A's parameter space
    if (typeB != "none")
        writeSlotParams(prefixA, typeB, valsB);

    // Step 3: Recreate modules with correct slot prefixes
    auto modA = eeval::EffectRegistry::createModule(typeA, prefixB);
    auto modB = (typeB != "none") ? eeval::EffectRegistry::createModule(typeB, prefixA) : nullptr;

    dspChain.setSlotTypeId(slotIndex, typeB);
    dspChain.setSlotTypeId(targetIndex, typeA);
    dspChain.setSlotModule(targetIndex, std::move(modA));
    dspChain.setSlotModule(slotIndex, std::move(modB));

    // Step 4: Update type choice parameters
    auto choices = eeval::EffectRegistry::getTypeChoices();
    auto* tpA = parameters.getParameter(prefixA + ".type");
    auto* tpB = parameters.getParameter(prefixB + ".type");
    if (tpA) {
        int ci = choices.indexOf(typeB);
        if (ci >= 0) tpA->setValueNotifyingHost(tpA->convertTo0to1((float)ci));
        else tpA->setValueNotifyingHost(0.0f);
    }
    if (tpB) {
        int ci = choices.indexOf(typeA);
        if (ci >= 0) tpB->setValueNotifyingHost(tpB->convertTo0to1((float)ci));
    }
}

std::vector<EasyEffectsAudioProcessor::SlotInfo> EasyEffectsAudioProcessor::getActiveSlots() const {
    std::vector<SlotInfo> result;
    for (int i = 0; i < eeval::EffectChain::MAX_SLOTS; ++i) {
        std::string typeId = dspChain.getSlotTypeId(i);
        if (typeId != "none") {
            const auto* desc = eeval::EffectRegistry::findType(typeId);
            std::string name = desc ? desc->displayName : typeId;
            result.push_back({i, typeId, name, true});
        }
    }
    return result;
}

// --- Standard AudioProcessor methods ---

const juce::String EasyEffectsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EasyEffectsAudioProcessor::acceptsMidi() const { return false; }
bool EasyEffectsAudioProcessor::producesMidi() const { return false; }
bool EasyEffectsAudioProcessor::isMidiEffect() const { return false; }
double EasyEffectsAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int EasyEffectsAudioProcessor::getNumPrograms() { return 1; }
int EasyEffectsAudioProcessor::getCurrentProgram() { return 0; }
void EasyEffectsAudioProcessor::setCurrentProgram(int) {}
const juce::String EasyEffectsAudioProcessor::getProgramName(int) { return {}; }
void EasyEffectsAudioProcessor::changeProgramName(int, const juce::String&) {}

void EasyEffectsAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    dspChain.prepare(spec);
}

void EasyEffectsAudioProcessor::releaseResources() {}

bool EasyEffectsAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void EasyEffectsAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Update parameter values from APVTS into DSP modules
    dspChain.updateParameters(parameters);

    // Execute DSP chain
    dspChain.process(buffer);

    // Push final output to FFT fifo for spectrum analyzer
    if (buffer.getNumChannels() > 0) {
        auto* channelDataL = buffer.getReadPointer(0);
        auto* channelDataR = buffer.getNumChannels() > 1 ? buffer.getReadPointer(1) : nullptr;
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            float sample = channelDataL[i];
            if (channelDataR != nullptr) sample = (sample + channelDataR[i]) * 0.5f;
            pushNextSampleIntoFifo(sample);
        }
    }
}

bool EasyEffectsAudioProcessor::hasEditor() const { return true; }

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
    auto state = parameters.copyState();

    // Serialize chain structure as a property
    juce::String chainStr;
    for (int i = 0; i < eeval::EffectChain::MAX_SLOTS; ++i) {
        std::string typeId = dspChain.getSlotTypeId(i);
        if (typeId != "none") {
            if (chainStr.isNotEmpty()) chainStr += ",";
            chainStr += juce::String(typeId);
        }
    }
    state.setProperty("CHAIN_ORDER", chainStr, nullptr);
    state.setProperty("SELECTED_EDITOR", selectedEditorIndex, nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml != nullptr)
        copyXmlToBinary(*xml, destData);
}

void EasyEffectsAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr) {
        if (xmlState->hasTagName(parameters.state.getType())) {
            auto newState = juce::ValueTree::fromXml(*xmlState);

            // Restore chain structure
            juce::String chainStr = newState.getProperty("CHAIN_ORDER", "");
            selectedEditorIndex = newState.getProperty("SELECTED_EDITOR", 0);

            parameters.replaceState(newState);

            // Rebuild chain from saved structure
            if (chainStr.isNotEmpty()) {
                // Clear existing chain
                for (int i = 0; i < eeval::EffectChain::MAX_SLOTS; ++i)
                    dspChain.clearSlot(i);

                juce::StringArray parts;
                parts.addTokens(chainStr, ",", "");
                for (int i = 0; i < parts.size() && i < eeval::EffectChain::MAX_SLOTS; ++i) {
                    std::string typeId = parts[i].toStdString();
                    std::string prefix = eeval::EffectRegistry::slotPrefix(i);
                    auto module = eeval::EffectRegistry::createModule(typeId, prefix);
                    if (module) {
                        dspChain.setSlotTypeId(i, typeId);
                        dspChain.setSlotModule(i, std::move(module));
                    }
                }
            }

            dspChain.updateParameters(parameters);
        }
    }
}

void EasyEffectsAudioProcessor::rebuildChainFromSlotTypes() {
    for (int i = 0; i < eeval::EffectChain::MAX_SLOTS; ++i) {
        std::string prefix = eeval::EffectRegistry::slotPrefix(i);
        auto* typeParam = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter(prefix + ".type"));
        if (typeParam) {
            int idx = typeParam->getIndex();
            auto choices = eeval::EffectRegistry::getTypeChoices();
            if (idx > 0 && idx < choices.size()) {
                std::string typeId = choices[idx].toStdString();
                if (dspChain.getSlotTypeId(i) != typeId) {
                    auto module = eeval::EffectRegistry::createModule(typeId, prefix);
                    if (module) {
                        dspChain.setSlotTypeId(i, typeId);
                        dspChain.setSlotModule(i, std::move(module));
                    }
                }
            } else {
                dspChain.clearSlot(i);
            }
        }
    }
}

// Plugin instantiation
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EasyEffectsAudioProcessor();
}
