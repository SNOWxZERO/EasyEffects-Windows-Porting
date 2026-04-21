#include "EffectRegistry.h"
#include "GainModule.h"
#include "CompressorModule.h"
#include "GateModule.h"
#include "EqualizerModule.h"
#include "LimiterModule.h"
#include "FilterModule.h"
#include "DelayModule.h"
#include "ReverbModule.h"
#include "ExciterModule.h"
#include "BassEnhancerModule.h"
#include "DeesserModule.h"
#include "ConvolverModule.h"
#include "StereoToolsModule.h"
#include "ExpanderModule.h"
#include "CrusherModule.h"
#include "MaximizerModule.h"

namespace eeval {

// Helper: create a module, set its slot param prefix, return it
template<typename T>
static std::unique_ptr<EffectModule> makeSlotModule(const std::string& slotPrefix, const std::string& typeId) {
    auto mod = std::make_unique<T>();
    mod->setParamPrefix(slotPrefix + "." + typeId);
    return mod;
}

std::vector<EffectTypeDescriptor> EffectRegistry::buildRegistry() {
    std::vector<EffectTypeDescriptor> types;

    // === Gate ===
    types.push_back({
        "gate", "Gate",
        {
            {"threshold", "Threshold", "dB", -60.0f, 0.0f, 0.1f, -40.0f},
            {"ratio",     "Ratio",     ":1", 1.0f, 20.0f, 0.1f, 10.0f},
            {"attack",    "Attack",    "ms", 0.1f, 100.0f, 0.1f, 1.0f},
            {"release",   "Release",   "ms", 1.0f, 1000.0f, 1.0f, 100.0f},
        },
        {}, // no choice params
        [](const std::string& prefix) { return makeSlotModule<GateModule>(prefix, "gate"); }
    });

    // === Compressor ===
    types.push_back({
        "compressor", "Compressor",
        {
            {"threshold", "Threshold", "dB", -60.0f, 0.0f, 0.1f, -10.0f},
            {"ratio",     "Ratio",     ":1", 1.0f, 100.0f, 0.1f, 3.0f},
            {"attack",    "Attack",    "ms", 0.1f, 100.0f, 0.1f, 2.0f},
            {"release",   "Release",   "ms", 1.0f, 1000.0f, 1.0f, 100.0f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<CompressorModule>(prefix, "compressor"); }
    });

    // === Equalizer ===
    {
        std::vector<SlotParamDescriptor> eqParams;
        for (int i = 0; i < EqualizerModule::NUM_BANDS; ++i) {
            std::string bandBase = "band" + std::to_string(i);
            float freqDefault = (i == 0) ? 100.0f : (i == 1) ? 1000.0f : (i == 2) ? 4000.0f : 10000.0f;
            eqParams.push_back({bandBase + ".gain", "Band " + std::to_string(i+1) + " Gain", "dB", -24.0f, 24.0f, 0.1f, 0.0f});
            eqParams.push_back({bandBase + ".freq", "Band " + std::to_string(i+1) + " Freq", "Hz", 20.0f, 20000.0f, 1.0f, freqDefault});
            eqParams.push_back({bandBase + ".q",    "Band " + std::to_string(i+1) + " Q",    "",   0.1f, 10.0f, 0.05f, 0.707f});
        }
        types.push_back({
            "eq", "Equalizer", eqParams, {},
            [](const std::string& prefix) { return makeSlotModule<EqualizerModule>(prefix, "eq"); }
        });
    }

    // === Deesser ===
    types.push_back({
        "deesser", "De-esser",
        {
            {"threshold", "Threshold",  "dB", -60.0f, 0.0f, 0.1f, -20.0f},
            {"ratio",     "Ratio",      ":1", 1.0f, 100.0f, 0.1f, 5.0f},
            {"frequency", "Split Freq", "Hz", 2000.0f, 20000.0f, 10.0f, 6000.0f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<DeesserModule>(prefix, "deesser"); }
    });

    // === Exciter ===
    types.push_back({
        "exciter", "Exciter",
        {
            {"amount",    "Amount",    "",   0.0f, 1.0f, 0.01f, 0.2f},
            {"harmonics", "Harmonics", "",   0.0f, 1.0f, 0.01f, 0.5f},
            {"cutoff",    "HP Cutoff", "Hz", 2000.0f, 20000.0f, 10.0f, 4000.0f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<ExciterModule>(prefix, "exciter"); }
    });

    // === Bass Enhancer ===
    types.push_back({
        "bassenhancer", "Bass Enhancer",
        {
            {"amount",    "Amount",    "",   0.0f, 1.0f, 0.01f, 0.2f},
            {"harmonics", "Harmonics", "",   0.0f, 1.0f, 0.01f, 0.5f},
            {"cutoff",    "LP Cutoff", "Hz", 20.0f, 1000.0f, 10.0f, 150.0f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<BassEnhancerModule>(prefix, "bassenhancer"); }
    });

    // === Filter ===
    types.push_back({
        "filter", "Filter",
        {
            {"cutoff",    "Cutoff",    "Hz", 20.0f, 20000.0f, 1.0f, 100.0f},
            {"resonance", "Resonance", "",   0.1f, 10.0f, 0.05f, 0.707f},
        },
        {
            {"type", "Type", juce::StringArray{"Highpass", "Lowpass"}, 0}
        },
        [](const std::string& prefix) { return makeSlotModule<FilterModule>(prefix, "filter"); }
    });

    // === Convolver ===
    types.push_back({
        "convolver", "Convolver",
        {}, // No float params beyond mix
        {},
        [](const std::string& prefix) { return makeSlotModule<ConvolverModule>(prefix, "convolver"); }
    });

    // === Delay ===
    types.push_back({
        "delay", "Delay",
        {
            {"time_ms",   "Time",         "ms", 0.0f, 2000.0f, 1.0f, 200.0f},
            {"feedback",  "Feedback",     "%",  0.0f, 100.0f, 1.0f, 30.0f},
            {"delay_mix", "Internal Mix", "%",  0.0f, 100.0f, 1.0f, 50.0f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<DelayModule>(prefix, "delay"); }
    });

    // === Reverb ===
    types.push_back({
        "reverb", "Reverb",
        {
            {"room_size", "Room Size", "", 0.0f, 1.0f, 0.01f, 0.5f},
            {"damping",   "Damping",   "", 0.0f, 1.0f, 0.01f, 0.5f},
            {"wet",       "Wet Level", "", 0.0f, 1.0f, 0.01f, 0.33f},
            {"dry",       "Dry Level", "", 0.0f, 1.0f, 0.01f, 0.4f},
            {"width",     "Width",     "", 0.0f, 1.0f, 0.01f, 1.0f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<ReverbModule>(prefix, "reverb"); }
    });

    // === Limiter ===
    types.push_back({
        "limiter", "Limiter",
        {
            {"threshold", "Threshold", "dB", -60.0f, 0.0f, 0.1f, -1.0f},
            {"release",   "Release",   "ms", 1.0f, 1000.0f, 1.0f, 100.0f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<LimiterModule>(prefix, "limiter"); }
    });

    // === Stereo Tools ===
    types.push_back({
        "stereotools", "Stereo Tools",
        {
            {"balance_in",      "Balance In",    "",   -1.0f,  1.0f, 0.01f, 0.0f},
            {"balance_out",     "Balance Out",   "",   -1.0f,  1.0f, 0.01f, 0.0f},
            {"side_level",      "Side Level",    "dB", -36.0f, 36.0f, 0.1f, 0.0f},
            {"side_balance",    "Side Balance",  "",   -1.0f,  1.0f, 0.01f, 0.0f},
            {"middle_level",    "Mid Level",     "dB", -36.0f, 36.0f, 0.1f, 0.0f},
            {"middle_panorama", "Mid Panorama",  "",   -1.0f,  1.0f, 0.01f, 0.0f},
            {"stereo_base",     "Stereo Base",   "",   -1.0f,  1.0f, 0.01f, 0.0f},
            {"delay",           "Delay",         "ms", -20.0f, 20.0f, 0.1f, 0.0f},
            {"stereo_phase",    "Stereo Phase",  "deg", 0.0f,  360.0f, 1.0f, 0.0f},
        },
        {
            {"mode",     "Mode",     juce::StringArray{"LR > LR", "LR > MS", "MS > LR", "LR > LL", "LR > RR", "LR > L+R", "LR > RL"}, 0},
            {"mute_l",   "Mute L",   juce::StringArray{"Off", "On"}, 0},
            {"mute_r",   "Mute R",   juce::StringArray{"Off", "On"}, 0},
            {"invert_l", "Invert L", juce::StringArray{"Off", "On"}, 0},
            {"invert_r", "Invert R", juce::StringArray{"Off", "On"}, 0}
        },
        [](const std::string& prefix) { return makeSlotModule<StereoToolsModule>(prefix, "stereotools"); }
    });

    // === Expander ===
    types.push_back({
        "expander", "Expander",
        {
            {"threshold",   "Threshold",   "dB", -60.0f,  0.0f,  0.1f, -40.0f},
            {"ratio",       "Ratio",       ":1",  1.0f,  20.0f,  0.1f,   2.0f},
            {"attack",      "Attack",      "ms",  0.1f, 100.0f,  0.1f,  10.0f},
            {"release",     "Release",     "ms", 10.0f, 1000.0f, 1.0f, 100.0f},
            {"knee",        "Knee",        "dB",  0.0f,  24.0f,  0.1f,   0.0f},
            {"makeup_gain", "Makeup Gain", "dB", -24.0f, 24.0f,  0.1f,   0.0f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<ExpanderModule>(prefix, "expander"); }
    });

    // === Crusher ===
    types.push_back({
        "crusher", "Crusher",
        {
            {"bits",    "Bit Depth", "",    1.0f,  16.0f, 0.1f,   16.0f},
            {"samples", "Samples",   "x",   1.0f, 100.0f, 1.0f,    1.0f},
            {"jitter",  "Jitter",    "",    0.0f,   1.0f, 0.01f,   0.0f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<CrusherModule>(prefix, "crusher"); }
    });

    // === Maximizer ===
    types.push_back({
        "maximizer", "Maximizer",
        {
            {"threshold", "Threshold", "dB", -60.0f, 0.0f, 0.1f, 0.0f},
            {"ceiling",   "Ceiling",   "dB", -20.0f, 0.0f, 0.1f, 0.0f},
            {"release",   "Release",   "ms",   1.0f, 1000.0f, 1.0f, 10.0f},
        },
        {
            {"soft_clip", "Soft Clip", juce::StringArray{"Off", "On"}, 0}
        },
        [](const std::string& prefix) { return makeSlotModule<MaximizerModule>(prefix, "maximizer"); }
    });

    // === Gain ===
    types.push_back({
        "gain", "Gain",
        {
            {"level", "Output", "dB", -24.0f, 24.0f, 0.1f, 0.0f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<GainModule>(prefix, "gain"); }
    });

    return types;
}

const std::vector<EffectTypeDescriptor>& EffectRegistry::getEffectTypes() {
    static auto types = buildRegistry();
    return types;
}

const EffectTypeDescriptor* EffectRegistry::findType(const std::string& typeId) {
    for (const auto& t : getEffectTypes()) {
        if (t.typeId == typeId)
            return &t;
    }
    return nullptr;
}

juce::StringArray EffectRegistry::getTypeChoices() {
    juce::StringArray choices;
    choices.add("none"); // Index 0 = empty slot
    for (const auto& t : getEffectTypes()) {
        choices.add(t.typeId);
    }
    return choices;
}

void EffectRegistry::registerSlotParameters(
    juce::AudioProcessorValueTreeState::ParameterLayout& layout,
    int slotIndex)
{
    std::string sp = slotPrefix(slotIndex); // "slot0", "slot1", etc.

    // Slot-level common parameters
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(sp + ".type", 1),
        "Slot " + std::to_string(slotIndex) + " Type",
        getTypeChoices(), 0)); // default: "none"

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(sp + ".bypass", 1),
        "Slot " + std::to_string(slotIndex) + " Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(sp + ".mix", 1),
        "Slot " + std::to_string(slotIndex) + " Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f));

    // Register all effect-type parameters for this slot
    for (const auto& effectType : getEffectTypes()) {
        std::string prefix = sp + "." + effectType.typeId; // e.g., "slot0.compressor"

        for (const auto& p : effectType.params) {
            std::string pid = prefix + "." + p.suffix;
            layout.add(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(pid, 1),
                p.label,
                juce::NormalisableRange<float>(p.minVal, p.maxVal, p.step),
                p.defaultVal));
        }

        for (const auto& c : effectType.choices) {
            std::string cid = prefix + "." + c.suffix;
            layout.add(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID(cid, 1),
                c.label,
                c.choices,
                c.defaultIndex));
        }
    }
}

std::unique_ptr<EffectModule> EffectRegistry::createModule(
    const std::string& typeId,
    const std::string& slotPfx)
{
    const auto* desc = findType(typeId);
    if (desc == nullptr || !desc->factory)
        return nullptr;
    return desc->factory(slotPfx);
}

} // namespace eeval
