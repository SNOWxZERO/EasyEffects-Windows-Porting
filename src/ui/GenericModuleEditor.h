#pragma once
#include <JuceHeader.h>
#include "../dsp/EffectRegistry.h"
#include "Theme.h"
#include "VisualMeterComponent.h"

namespace eeval {
namespace ui {

/**
 * GenericModuleEditor - Standard UI for most effects.
 * Automatically generates sliders/comboboxes from the registry.
 */
class GenericModuleEditor : public juce::Component, public juce::Timer {
public:
    GenericModuleEditor(juce::AudioProcessorValueTreeState& stateToUse,
                        int slotIdx,
                        const std::string& typeId,
                        const std::string& dispName);

    ~GenericModuleEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    int slotIndex;
    std::string effectTypeId;
    std::string displayName;

    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::Label> labels;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> attachments;

    juce::OwnedArray<juce::ComboBox> comboBoxes;
    juce::OwnedArray<juce::Label> choiceLabels;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAttachments;

    VisualMeterComponent grMeter { true };
    VisualMeterComponent vadMeter { false };
    bool showGrMeter = false;
    bool showVadMeter = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenericModuleEditor)
};

} // namespace ui
} // namespace eeval
