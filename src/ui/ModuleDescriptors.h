#pragma once
#include <JuceHeader.h>
#include <string>
#include <vector>

namespace eeval {

// Struct describing an individual parameter layout
struct ParamDescriptor {
    std::string id;       // The full APVTS ID (e.g., "compressor.threshold")
    std::string label;    // Human readable text
    std::string unit;     // "dB", "ms", "%", "Hz"
};

// Provides static Layout maps for all modules avoiding string parsing
class ModuleDescriptors {
public:
    static std::vector<ParamDescriptor> getParametersForModule(const std::string& moduleId) {
        if (moduleId == "gate") {
            return {
                {"gate.threshold", "Threshold", "dB"},
                {"gate.ratio", "Ratio", ":1"},
                {"gate.attack", "Attack", "ms"},
                {"gate.release", "Release", "ms"},
                {"gate.mix", "Mix", "%"}
            };
        }
        else if (moduleId == "compressor") {
            return {
                {"compressor.threshold", "Threshold", "dB"},
                {"compressor.ratio", "Ratio", ":1"},
                {"compressor.attack", "Attack", "ms"},
                {"compressor.release", "Release", "ms"},
                {"compressor.mix", "Mix", "%"}
            };
        }
        else if (moduleId == "eq") {
            // Because Equalizer has so many bands, we manually specify them
            std::vector<ParamDescriptor> p;
            for (int i = 0; i < 4; ++i) { // NUM_BANDS=4 matching EqualizerModule
                std::string base = "eq.band" + std::to_string(i);
                p.push_back({base + ".gain", "Band " + std::to_string(i+1) + " Gain", "dB"});
                p.push_back({base + ".freq", "Band " + std::to_string(i+1) + " Freq", "Hz"});
                p.push_back({base + ".q",    "Band " + std::to_string(i+1) + " Q", ""});
            }
            p.push_back({"eq.mix", "Mix", "%"});
            return p;
        }
        else if (moduleId == "deesser") {
            return {
                {"deesser.threshold", "Threshold", "dB"},
                {"deesser.ratio", "Ratio", ":1"},
                {"deesser.frequency", "Split Freq", "Hz"},
                {"deesser.mix", "Mix", "%"}
            };
        }
        else if (moduleId == "exciter") {
            return {
                {"exciter.amount", "Amount", ""},
                {"exciter.harmonics", "Harmonics", ""},
                {"exciter.cutoff", "HP Cutoff", "Hz"},
                {"exciter.mix", "Mix", "%"}
            };
        }
        else if (moduleId == "bassenhancer") {
            return {
                {"bassenhancer.amount", "Amount", ""},
                {"bassenhancer.harmonics", "Harmonics", ""},
                {"bassenhancer.cutoff", "LP Cutoff", "Hz"},
                {"bassenhancer.mix", "Mix", "%"}
            };
        }
        else if (moduleId == "limiter") {
            return {
                {"limiter.threshold", "Threshold", "dB"},
                {"limiter.release", "Release", "ms"},
                {"limiter.mix", "Mix", "%"}
            };
        }
        else if (moduleId == "filter") {
            return {
                {"filter.cutoff", "Cutoff", "Hz"},
                {"filter.resonance", "Resonance", ""},
                {"filter.mix", "Mix", "%"}
            };
            // Note: filter.type is a Choice parameter, handled specifically
        }
        else if (moduleId == "delay") {
            return {
                {"delay.time_ms", "Time", "ms"},
                {"delay.feedback", "Feedback", "%"},
                {"delay.delay_mix", "Internal Mix", "%"},
                {"delay.mix", "Module Mix", "%"}
            };
        }
        else if (moduleId == "reverb") {
            return {
                {"reverb.room_size", "Room Size", ""},
                {"reverb.damping", "Damping", ""},
                {"reverb.wet", "Wet Level", ""},
                {"reverb.dry", "Dry Level", ""},
                {"reverb.width", "Width", ""},
                {"reverb.mix", "Mix", "%"}
            };
        }
        else if (moduleId == "gain") {
            return {
                {"gain.level", "Output", "dB"},
                {"gain.mix", "Mix", "%"}
            };
        }
        
        return {}; // Unknown or no standard parameters (e.g., LevelMeter, Convolver)
    }
};

} // namespace eeval
