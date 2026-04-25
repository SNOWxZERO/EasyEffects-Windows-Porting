#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace eeval {
namespace ui {

/**
 * DynamicsVisualizer - Renders the transfer function (Input vs Output)
 * for Compressor, Gate, and Expander modules. Includes a live signal indicator.
 */
class DynamicsVisualizer : public juce::Component, public juce::Timer {
public:
    DynamicsVisualizer(juce::AudioProcessorValueTreeState& vts, const std::string& prefix, const std::string& type);
    ~DynamicsVisualizer() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // Call this from the parent to update the current signal level (ball position)
    void setCurrentLevels(float inputLevelDb, float outputLevelDb);

private:
    juce::AudioProcessorValueTreeState& apvts;
    std::string paramPrefix;
    std::string effectType;

    juce::Path staticCurve;
    float lastInputDb = -100.0f;
    float lastOutputDb = -100.0f;

    void updateStaticCurve();
    
    // Parameter values
    float threshold = 0.0f;
    float ratio = 1.0f;
    float knee = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynamicsVisualizer)
};

} // namespace ui
} // namespace eeval
