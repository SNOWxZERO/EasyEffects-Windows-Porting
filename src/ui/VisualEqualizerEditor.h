#pragma once

#include "GenericModuleEditor.h"
#include "EQPlotComponent.h"
#include "EQNodeComponent.h"
#include <vector>

namespace eeval {
namespace ui {

/**
 * VisualEqualizerEditor - SteelSeries-style interactive EQ editor.
 */
class VisualEqualizerEditor : public juce::Component {
public:
    VisualEqualizerEditor(juce::AudioProcessorValueTreeState& vts, int slotIndex);
    ~VisualEqualizerEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    int slotIdx;

    EQPlotComponent plot;
    std::vector<std::unique_ptr<EQNodeComponent>> nodes;
    
    juce::TextButton addBandBtn { "+ Add Band" };
    juce::Label titleLabel;

    void addNextBand();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualEqualizerEditor)
};

} // namespace ui
} // namespace eeval
