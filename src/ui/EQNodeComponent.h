#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace eeval {
namespace ui {

class EQPlotComponent;

/**
 * EQNodeComponent - Interactive handle for an EQ band.
 */
class EQNodeComponent : public juce::Component {
public:
    EQNodeComponent(juce::AudioProcessorValueTreeState& vts, int slotIndex, int bandIndex, EQPlotComponent& plot);

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    void updatePosition();

private:
    juce::AudioProcessorValueTreeState& apvts;
    int slotIndex;
    int bandIdx;
    std::string prefix;
    EQPlotComponent& parentPlot;

    juce::ComponentDragger dragger;
    
    // Cached param pointers
    std::atomic<float>* freqParam = nullptr;
    std::atomic<float>* gainParam = nullptr;
    std::atomic<float>* enabledParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQNodeComponent)
};

} // namespace ui
} // namespace eeval
