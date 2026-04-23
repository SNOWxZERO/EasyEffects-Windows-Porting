#include "GenericModuleEditor.h"
#include "VisualMeterComponent.h"
#include "../PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace eeval {
namespace ui {

GenericModuleEditor::GenericModuleEditor(juce::AudioProcessorValueTreeState& stateToUse,
                                         int slotIdx,
                                         const std::string& typeId,
                                         const std::string& dispName)
    : apvts(stateToUse), slotIndex(slotIdx), effectTypeId(typeId), displayName(dispName)
{
    std::string prefix = "slot" + std::to_string(slotIndex) + "." + effectTypeId;

    const auto* desc = EffectRegistry::findType(effectTypeId);
    if (!desc) return;

    for (const auto& pdesc : desc->params) {
        std::string fullId = prefix + "." + pdesc.suffix;
        auto* slider = new juce::Slider(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
        slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 28);
        slider->setTextValueSuffix(" " + pdesc.unit);
        slider->setColour(juce::Slider::textBoxOutlineColourId, theme::borderSubtle);
        slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF1A1A1A));

        auto* label = new juce::Label(fullId + "_label", pdesc.label);
        label->setJustificationType(juce::Justification::centredLeft);
        label->setColour(juce::Label::textColourId, theme::textSecondary);

        addAndMakeVisible(slider);
        addAndMakeVisible(label);
        sliders.add(slider);
        labels.add(label);

        attachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, fullId, *slider));
    }

    for (const auto& cdesc : desc->choices) {
        std::string fullId = prefix + "." + cdesc.suffix;
        auto* combo = new juce::ComboBox();
        for (int i = 0; i < (int)cdesc.choices.size(); ++i)
            combo->addItem(cdesc.choices[i], i + 1);
        combo->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(combo);

        auto* label = new juce::Label(fullId + "_label", cdesc.label);
        label->setJustificationType(juce::Justification::centredLeft);
        label->setColour(juce::Label::textColourId, theme::textSecondary);
        addAndMakeVisible(label);

        choiceLabels.add(label);
        comboBoxes.add(combo);
        comboAttachments.add(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(apvts, fullId, *combo));
    }

    // Add Mix slider
    if (effectTypeId != "levelmeter") {
        std::string mixId = "slot" + std::to_string(slotIndex) + ".mix";
        auto* mixSlider = new juce::Slider(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
        mixSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 28);
        mixSlider->setTextValueSuffix(" %");
        addAndMakeVisible(mixSlider);
        sliders.add(mixSlider);
        labels.add(new juce::Label(mixId + "_label", "Dry/Wet Mix"));
        addAndMakeVisible(labels.getLast());
        attachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, mixId, *mixSlider));
    }

    // Determine if we need meters
    showGrMeter = (effectTypeId == "compressor" || effectTypeId == "gate" || effectTypeId == "expander");
    showVadMeter = (effectTypeId == "rnnoise");

    if (showGrMeter) addAndMakeVisible(grMeter);
    if (showVadMeter) addAndMakeVisible(vadMeter);

    startTimerHz(30); // 30 FPS updates
}

void GenericModuleEditor::timerCallback() {
    auto* processor = dynamic_cast<EasyEffectsAudioProcessor*>(&apvts.processor);
    if (!processor) return;

    auto* module = processor->getChain().getSlotModule(slotIndex);
    if (!module) return;

    if (showGrMeter) grMeter.setValue(module->getGainReduction());
    if (showVadMeter) vadMeter.setValue(module->getVADProbability());
}

void GenericModuleEditor::paint(juce::Graphics& g) {
    auto area = getLocalBounds();
    auto titleArea = area.removeFromTop(40);
    g.setColour(theme::textPrimary);
    g.setFont(18.0f);
    g.drawText(displayName, titleArea.reduced(20, 0), juce::Justification::centredLeft, true);

    auto cardArea = area.reduced(15, 5);
    g.setColour(theme::bgCard);
    g.fillRoundedRectangle(cardArea.toFloat(), 8.0f);
    g.setColour(theme::borderSubtle);
    g.drawRoundedRectangle(cardArea.toFloat(), 8.0f, 1.0f);
}

void GenericModuleEditor::resized() {
    auto area = getLocalBounds();
    area.removeFromTop(40);
    auto cardArea = area.reduced(15, 5);
    
    if (showGrMeter || showVadMeter) {
        auto meterArea = cardArea.removeFromRight(25).reduced(5, 40);
        if (showGrMeter) grMeter.setBounds(meterArea);
        if (showVadMeter) vadMeter.setBounds(meterArea);
    }

    auto innerArea = cardArea.reduced(15, 35);
    int x = innerArea.getX();
    int y = innerArea.getY();
    int rowHeight = 35;
    int labelWidth = 120;
    int sliderWidth = innerArea.getWidth() - labelWidth - 10;

    for (int i = 0; i < comboBoxes.size(); ++i) {
        choiceLabels[i]->setBounds(x, y, labelWidth, rowHeight);
        comboBoxes[i]->setBounds(x + labelWidth + 10, y, juce::jmin(sliderWidth, 200), rowHeight);
        y += rowHeight + 8;
    }

    for (int i = 0; i < sliders.size(); ++i) {
        labels[i]->setBounds(x, y, labelWidth, rowHeight);
        sliders[i]->setBounds(x + labelWidth + 10, y, sliderWidth, rowHeight);
        y += rowHeight + 8;
    }
}

} // namespace ui
} // namespace eeval
