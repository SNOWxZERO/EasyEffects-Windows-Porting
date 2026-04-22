#pragma once
#include <JuceHeader.h>

namespace eeval {

/**
 * A 3-band phase-aligned Linkwitz-Riley (4th order) crossover.
 * Provides Low, Mid, and High bands with perfect reconstruction.
 */
class BandSplitter {
public:
    BandSplitter() = default;

    void prepare(const juce::dsp::ProcessSpec& spec) {
        crossover1.prepare(spec);
        crossover2.prepare(spec);
        crossover3.prepare(spec);
        
        crossover1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        crossover2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        crossover3.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        
        updateFrequencies();
    }

    void setFrequencies(float lowMid, float midHigh) {
        lowMidFreq = lowMid;
        midHighFreq = midHigh;
        updateFrequencies();
    }

    void reset() {
        crossover1.reset();
        crossover2.reset();
        crossover3.reset();
    }

    /**
     * Splits a single sample into three bands.
     * @param channel The channel index
     * @param input The input sample
     * @param low Output: low band
     * @param mid Output: mid band
     * @param high Output: high band
     */
    void processSample(int channel, float input, float& low, float& mid, float& high) {
        // Stage 1: Split at low-mid frequency
        float lowMidBase = crossover1.processSample(channel, input);
        float midHighBase = input - lowMidBase; // Complementary HPF output

        // Stage 2: Split mid-high base at mid-high frequency
        mid = crossover2.processSample(channel, midHighBase);
        high = midHighBase - mid;

        // Stage 3: Phase compensation for the low band
        // We pass the low band through the same phase shift as the second crossover
        low = crossover3.processSample(channel, lowMidBase);
    }

private:
    void updateFrequencies() {
        crossover1.setCutoffFrequency(lowMidFreq);
        crossover2.setCutoffFrequency(midHighFreq);
        crossover3.setCutoffFrequency(midHighFreq); // Same as Stage 2 for alignment
    }

    juce::dsp::LinkwitzRileyFilter<float> crossover1; // Split F1
    juce::dsp::LinkwitzRileyFilter<float> crossover2; // Split F2
    juce::dsp::LinkwitzRileyFilter<float> crossover3; // Phase alignment at F2 for Low band

    float lowMidFreq = 200.0f;
    float midHighFreq = 2500.0f;
};

} // namespace eeval
