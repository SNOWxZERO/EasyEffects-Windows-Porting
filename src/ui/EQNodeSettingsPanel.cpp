#include "EQNodeSettingsPanel.h"
#include "../dsp/EffectRegistry.h"
#include "Theme.h"

namespace eeval {
namespace ui {

EQNodeSettingsPanel::EQNodeSettingsPanel(juce::AudioProcessorValueTreeState& vts, int slotIndex, int bandIndex)
    : apvts(vts), bandIdx(bandIndex)
{
    prefix = eeval::EffectRegistry::slotPrefix(slotIndex) + ".eq.band" + std::to_string(bandIdx) + ".";

    setSize(220, 140);

    addAndMakeVisible(enabledButton);
    enabledAttach = std::make_unique<ButtonAttachment>(apvts, prefix + "enabled", enabledButton);

    juce::StringArray filterTypes {"Peaking", "Low Pass", "High Pass", "Low Shelf", "High Shelf", "Notch"};
    typeBox.addItemList(filterTypes, 1);
    addAndMakeVisible(typeBox);
    typeAttach = std::make_unique<BoxAttachment>(apvts, prefix + "type", typeBox);

    auto setupSlider = [&](juce::Slider& s, juce::Label& l, const std::string& suffix, const std::string& unit) {
        s.setTextValueSuffix(unit);
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
        addAndMakeVisible(s);
        
        l.setJustificationType(juce::Justification::centred);
        l.setColour(juce::Label::textColourId, theme::textSecondary);
        l.setFont(10.0f);
        addAndMakeVisible(l);
        
        return std::make_unique<Attachment>(apvts, prefix + suffix, s);
    };

    freqAttach = setupSlider(freqSlider, freqLabel, "freq", " Hz");
    gainAttach = setupSlider(gainSlider, gainLabel, "gain", " dB");
    qAttach    = setupSlider(qSlider, qLabel, "q", "");

    // Force numerical entry only style
    freqSlider.setSliderStyle(juce::Slider::IncDecButtons);
    gainSlider.setSliderStyle(juce::Slider::IncDecButtons);
    qSlider.setSliderStyle(juce::Slider::IncDecButtons);
}

void EQNodeSettingsPanel::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xEE111111));
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 8.0f, 1.0f);
}

void EQNodeSettingsPanel::resized() {
    auto area = getLocalBounds().reduced(10);
    
    auto topRow = area.removeFromTop(24);
    enabledButton.setBounds(topRow.removeFromLeft(80));
    typeBox.setBounds(topRow.reduced(2, 0));

    area.removeFromTop(10);
    
    auto sliderArea = area;
    int w = sliderArea.getWidth() / 3;
    
    auto fArea = sliderArea.removeFromLeft(w);
    freqSlider.setBounds(fArea.removeFromTop(40));
    freqLabel.setBounds(fArea);

    auto gArea = sliderArea.removeFromLeft(w);
    gainSlider.setBounds(gArea.removeFromTop(40));
    gainLabel.setBounds(gArea);

    auto qArea = sliderArea;
    qSlider.setBounds(qArea.removeFromTop(40));
    qLabel.setBounds(qArea);
}

} // namespace ui
} // namespace eeval
