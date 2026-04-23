#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>

namespace eeval {
namespace ui {

/**
 * EQPlotComponent - Logarithmic frequency response visualizer.
 * Draws the cumulative magnitude response of all EQ bands.
 */
class EQPlotComponent : public juce::Component, 
                         public juce::AudioProcessorValueTreeState::Listener {
public:
    EQPlotComponent(juce::AudioProcessorValueTreeState& vts, int slotIndex);
    ~EQPlotComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Trigger a re-calculation of the curve
    void updateResponse();

    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    int slotIdx;
    std::string prefix;

    juce::Path responsePath;
    
    // Cached response values for the current view
    std::vector<float> magnitudes;
    
    bool needsUpdate = true;
    double sampleRate = 48000.0;

    // Helpers
    float getXForFreq(float freq) const;
    float getFreqForX(float x) const;
    float getYForMag(float mag) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQPlotComponent)
};

} // namespace ui
} // namespace eeval
