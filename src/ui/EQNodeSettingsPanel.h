#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace eeval {
namespace ui {

/**
 * EQNodeSettingsPanel - The floating content for the EQ Node Inspector.
 * Contains numerical inputs and type selectors.
 */
class EQNodeSettingsPanel : public juce::Component {
public:
    EQNodeSettingsPanel(juce::AudioProcessorValueTreeState& vts, int slotIndex, int bandIndex);
    ~EQNodeSettingsPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& apvts;
    int bandIdx;
    std::string prefix;

    juce::ToggleButton enabledButton { "Enabled" };
    juce::ComboBox typeBox;
    
    juce::Slider freqSlider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxBelow };
    juce::Slider gainSlider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxBelow };
    juce::Slider qSlider    { juce::Slider::LinearHorizontal, juce::Slider::TextBoxBelow };

    juce::Label freqLabel { "freqLabel", "Freq" };
    juce::Label gainLabel { "gainLabel", "Gain" };
    juce::Label qLabel    { "qLabel", "Q" };

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using BoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<ButtonAttachment> enabledAttach;
    std::unique_ptr<BoxAttachment> typeAttach;
    std::unique_ptr<Attachment> freqAttach;
    std::unique_ptr<Attachment> gainAttach;
    std::unique_ptr<Attachment> qAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQNodeSettingsPanel)
};

} // namespace ui
} // namespace eeval
