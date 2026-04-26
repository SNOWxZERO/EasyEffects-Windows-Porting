#pragma once

#include "GenericModuleEditor.h"
#include "EQPlotComponent.h"
#include "EQNodeComponent.h"
#include <vector>

class EasyEffectsAudioProcessor;

namespace eeval {
namespace ui {

/**
 * VisualEqualizerEditor - SteelSeries-style interactive EQ editor.
 * Now includes per-module preset support.
 */
class VisualEqualizerEditor : public juce::Component {
public:
    VisualEqualizerEditor(EasyEffectsAudioProcessor& p, juce::AudioProcessorValueTreeState& vts, int slotIndex);
    ~VisualEqualizerEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    EasyEffectsAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& apvts;
    int slotIdx;

    EQPlotComponent plot;
    std::vector<std::unique_ptr<EQNodeComponent>> nodes;
    
    juce::TextButton addBandBtn { "+ Add Band" };
    juce::TextButton presetBtn  { "Presets" };
    juce::Label titleLabel;

    void addNextBand();
    void showPresetMenu();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualEqualizerEditor)
};

} // namespace ui
} // namespace eeval
