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
#include "CrossfeedModule.h"
#include "BassLoudnessModule.h"
#include "CrystalizerModule.h"
#include "AutoGainModule.h"
#include "MultibandCompressorModule.h"
#include "MultibandGateModule.h"
#include "ChorusModule.h"
#include "PhaserModule.h"
#include "RNNoiseModule.h"
#include "PitchShiftModule.h"

namespace eeval {

// Helper: create a module, set its slot param prefix, return it
template<typename T>
static std::unique_ptr<EffectModule> makeSlotModule(const std::string& slotPrefix, const std::string& typeId) {
    auto mod = std::make_unique<T>();
    mod->setSlotPrefix(slotPrefix);
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
        std::vector<SlotChoiceDescriptor> eqChoices;
        
        juce::StringArray filterTypes {"Peaking", "Low Pass", "High Pass", "Low Shelf", "High Shelf", "Notch"};

        for (int i = 0; i < EqualizerModule::NUM_BANDS; ++i) {
            std::string bandBase = "band" + std::to_string(i);
            
            // Smart defaults for a 10-band setup
            float freqDefault = 1000.0f;
            int typeDefault = 0; // Peaking
            float enabledDefault = 1.0f;

            if (i == 0) { freqDefault = 80.0f; typeDefault = 2; } // High Pass
            else if (i == 1) { freqDefault = 150.0f; }
            else if (i == 2) { freqDefault = 400.0f; }
            else if (i == 3) { freqDefault = 800.0f; }
            else if (i == 4) { freqDefault = 1500.0f; }
            else if (i == 5) { freqDefault = 3000.0f; }
            else if (i == 6) { freqDefault = 5000.0f; }
            else if (i == 7) { freqDefault = 8000.0f; }
            else if (i == 8) { freqDefault = 12000.0f; }
            else if (i == 9) { freqDefault = 16000.0f; typeDefault = 1; } // Low Pass

            // Disable bands 2-9 by default to keep it clean initially
            if (i > 0 && i < 9) enabledDefault = 0.0f;

            eqParams.push_back({bandBase + ".enabled", "Band " + std::to_string(i+1) + " Enabled", "", 0.0f, 1.0f, 1.0f, enabledDefault});
            eqParams.push_back({bandBase + ".gain", "Band " + std::to_string(i+1) + " Gain", "dB", -24.0f, 24.0f, 0.1f, 0.0f});
            eqParams.push_back({bandBase + ".freq", "Band " + std::to_string(i+1) + " Freq", "Hz", 20.0f, 20000.0f, 1.0f, freqDefault});
            eqParams.push_back({bandBase + ".q",    "Band " + std::to_string(i+1) + " Q",    "",   0.1f, 10.0f, 0.05f, 0.707f});
            
            eqChoices.push_back({bandBase + ".type", "Band " + std::to_string(i+1) + " Type", filterTypes, typeDefault});
        }

        types.push_back({
            "eq", "Equalizer", eqParams, eqChoices,
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

    // === Crossfeed ===
    types.push_back({
        "crossfeed", "Crossfeed",
        {
            {"fcut", "Cutoff", "Hz", 300.0f, 2000.0f, 10.0f, 700.0f},
            {"feed", "Feed",   "dB",   1.0f,   15.0f, 0.1f,   4.5f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<CrossfeedModule>(prefix, "crossfeed"); }
    });

    // === Bass Loudness ===
    types.push_back({
        "bass_loudness", "Bass Loudness",
        {
            {"loudness", "Loudness", "dB",  0.0f, 30.0f, 0.1f, 0.0f},
            {"output",   "Output",   "dB", -36.0f, 36.0f, 0.1f, 0.0f},
        },
        {
            {"link", "Link", juce::StringArray{"Off", "On"}, 0}
        },
        [](const std::string& prefix) { return makeSlotModule<BassLoudnessModule>(prefix, "bass_loudness"); }
    });

    // === Crystalizer ===
    types.push_back({
        "crystalizer", "Crystalizer",
        {
            {"intensity_low",  "Intensity Low",  "", 0.0f, 16.0f, 0.1f, 0.0f},
            {"intensity_mid",  "Intensity Mid",  "", 0.0f, 16.0f, 0.1f, 0.0f},
            {"intensity_high", "Intensity High", "", 0.0f, 16.0f, 0.1f, 0.0f},
        },
        {
            {"adaptive_intensity", "Adaptive", juce::StringArray{"Off", "On"}, 1}
        },
        [](const std::string& prefix) { return makeSlotModule<CrystalizerModule>(prefix, "crystalizer"); }
    });

    // === Auto Gain ===
    types.push_back({
        "autogain", "Auto Gain",
        {
            {"target",            "Target",            "LUFS", -100.0f, 0.0f,    1.0f, -23.0f},
            {"attack",            "Attack",            "ms",     10.0f, 2000.0f, 10.0f, 100.0f},
            {"release",           "Release",           "ms",     10.0f, 5000.0f, 10.0f, 500.0f},
            {"silence_threshold", "Silence Threshold", "dB",   -100.0f, 0.0f,    1.0f, -60.0f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<AutoGainModule>(prefix, "autogain"); }
    });

    // === Multiband Compressor ===
    {
        std::vector<SlotParamDescriptor> mbParams;
        mbParams.push_back({"crossover_low",  "Crossover Low",  "Hz", 20.0f, 1000.0f,  10.0f, 200.0f});
        mbParams.push_back({"crossover_high", "Crossover High", "Hz", 1000.0f, 20000.0f, 100.0f, 2500.0f});
        
        const char* bands[3] = { "low", "mid", "high" };
        for (const char* b : bands) {
            std::string s = b;
            std::string label = (s == "low" ? "Low" : s == "mid" ? "Mid" : "High");
            mbParams.push_back({"threshold_" + s, label + " Threshold", "dB", -60.0f, 0.0f, 0.1f, -10.0f});
            mbParams.push_back({"ratio_" + s,     label + " Ratio",     ":1",  1.0f, 20.0f, 0.1f, 3.0f});
            mbParams.push_back({"attack_" + s,    label + " Attack",    "ms",  0.1f, 100.0f, 0.1f, 2.0f});
            mbParams.push_back({"release_" + s,   label + " Release",   "ms",  1.0f, 1000.0f, 1.0f, 100.0f});
            mbParams.push_back({"gain_" + s,      label + " Gain",      "dB", -24.0f, 24.0f, 0.1f, 0.0f});
        }
        
        types.push_back({
            "multiband_compressor", "Multiband Compressor", mbParams, {},
            [](const std::string& prefix) { return makeSlotModule<MultibandCompressorModule>(prefix, "multiband_compressor"); }
        });
    }

    // === Multiband Gate ===
    {
        std::vector<SlotParamDescriptor> mbgParams;
        mbgParams.push_back({"crossover_low",  "Crossover Low",  "Hz", 20.0f, 1000.0f,  10.0f, 200.0f});
        mbgParams.push_back({"crossover_high", "Crossover High", "Hz", 1000.0f, 20000.0f, 100.0f, 2500.0f});
        
        const char* bands[3] = { "low", "mid", "high" };
        for (const char* b : bands) {
            std::string s = b;
            std::string label = (s == "low" ? "Low" : s == "mid" ? "Mid" : "High");
            mbgParams.push_back({"threshold_" + s, label + " Threshold", "dB", -60.0f, 0.0f, 0.1f, -40.0f});
            mbgParams.push_back({"ratio_" + s,     label + " Ratio",     ":1",  1.0f, 20.0f, 0.1f, 10.0f});
            mbgParams.push_back({"attack_" + s,    label + " Attack",    "ms",  0.1f, 100.0f, 0.1f, 1.0f});
            mbgParams.push_back({"release_" + s,   label + " Release",   "ms",  1.0f, 1000.0f, 1.0f, 100.0f});
        }

        types.push_back({
            "multiband_gate", "Multiband Gate", mbgParams, {},
            [](const std::string& prefix) { return makeSlotModule<MultibandGateModule>(prefix, "multiband_gate"); }
        });
    }

    // === Chorus ===
    types.push_back({
        "chorus", "Chorus",
        {
            {"rate",         "Rate",         "Hz", 0.01f, 20.0f, 0.01f, 1.0f},
            {"depth",        "Depth",        "",   0.0f,  1.0f,  0.01f, 0.5f},
            {"feedback",     "Feedback",     "",  -1.0f,  1.0f,  0.01f, 0.0f},
            {"centre_delay", "Centre Delay", "ms", 1.0f, 100.0f, 0.1f, 7.0f},
            {"chorus_mix",   "Chorus Mix",   "",   0.0f,  1.0f,  0.01f, 0.5f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<ChorusModule>(prefix, "chorus"); }
    });

    // === Phaser ===
    types.push_back({
        "phaser", "Phaser",
        {
            {"rate",        "Rate",        "Hz", 0.01f, 20.0f, 0.01f, 0.5f},
            {"depth",       "Depth",       "",   0.0f,  1.0f,  0.01f, 0.5f},
            {"feedback",    "Feedback",    "",  -1.0f,  1.0f,  0.01f, 0.5f},
            {"centre_freq", "Centre Freq", "Hz", 20.0f, 10000.0f, 10.0f, 1000.0f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<PhaserModule>(prefix, "phaser"); }
    });

    // === Noise Reduction (RNNoise) ===
    types.push_back({
        "rnnoise", "Noise Reduction",
        {
            {"enabled", "Enabled", "", 0.0f, 1.0f, 1.0f, 1.0f},
            {"vad_threshold", "VAD Threshold", "%", 0.0f, 1.0f, 0.01f, 0.5f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<RNNoiseModule>(prefix, "rnnoise"); }
    });
    // === Pitch Shift (SoundTouch) ===
    types.push_back({
        "pitch_shift", "Pitch Shift",
        {
            {"semitones", "Semitones", "st", -12.0f, 12.0f, 0.1f, 0.0f},
        },
        {},
        [](const std::string& prefix) { return makeSlotModule<PitchShiftModule>(prefix, "pitch_shift"); }
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
