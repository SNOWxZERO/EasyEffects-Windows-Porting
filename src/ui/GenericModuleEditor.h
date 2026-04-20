#pragma once
#include <JuceHeader.h>
#include "../dsp/EffectRegistry.h"
#include "Theme.h"

namespace eeval {
namespace ui {

class GenericModuleEditor : public juce::Component {
public:
    GenericModuleEditor(juce::AudioProcessorValueTreeState& stateToUse,
                        int slotIdx,
                        const std::string& typeId,
                        const std::string& dispName)
        : apvts(stateToUse), slotIndex(slotIdx), effectTypeId(typeId), displayName(dispName)
    {
        std::string prefix = "slot" + std::to_string(slotIndex) + "." + effectTypeId;

        // Get parameter descriptors from the registry
        const auto* desc = EffectRegistry::findType(effectTypeId);
        if (!desc) return;

        for (const auto& pdesc : desc->params) {
            std::string fullId = prefix + "." + pdesc.suffix;

            juce::Slider* slider = nullptr;

            if (effectTypeId == "eq" && pdesc.suffix.find("gain") != std::string::npos) {
                slider = new juce::Slider(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
                slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 20);
            } else {
                slider = new juce::Slider(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
                slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 28);
            }

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

            attachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(
                apvts, fullId, *slider));
        }

        // Choice parameters (e.g., filter type)
        for (const auto& cdesc : desc->choices) {
            std::string fullId = prefix + "." + cdesc.suffix;

            auto* combo = new juce::ComboBox();
            for (int i = 0; i < cdesc.choices.size(); ++i)
                combo->addItem(cdesc.choices[i], i + 1);
            combo->setJustificationType(juce::Justification::centred);
            addAndMakeVisible(combo);

            auto* label = new juce::Label(fullId + "_label", cdesc.label);
            label->setJustificationType(juce::Justification::centredLeft);
            label->setColour(juce::Label::textColourId, theme::textSecondary);
            addAndMakeVisible(label);

            choiceLabels.add(label);
            comboBoxes.add(combo);
            comboAttachments.add(
                new juce::AudioProcessorValueTreeState::ComboBoxAttachment(apvts, fullId, *combo));
        }
    }

    ~GenericModuleEditor() override = default;

    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds();

        // Module title
        auto titleArea = area.removeFromTop(40);
        g.setColour(theme::textPrimary);
        g.setFont(18.0f);
        g.drawText(displayName, titleArea.reduced(20, 0), juce::Justification::centredLeft, true);

        // "Controls" card
        auto cardArea = area.reduced(15, 5);
        g.setColour(theme::bgCard);
        g.fillRoundedRectangle(cardArea.toFloat(), 8.0f);
        g.setColour(theme::borderSubtle);
        g.drawRoundedRectangle(cardArea.toFloat(), 8.0f, 1.0f);
        g.setColour(theme::textPrimary);
        g.setFont(14.0f);
        g.drawText("Controls", cardArea.getX() + 15, cardArea.getY() + 8, 100, 20,
                   juce::Justification::centredLeft);
    }

    void resized() override {
        auto area = getLocalBounds();
        area.removeFromTop(40);
        auto cardArea = area.reduced(15, 5);
        auto innerArea = cardArea.reduced(15, 35);

        int x = innerArea.getX();
        int y = innerArea.getY();

        bool isEq = (effectTypeId == "eq");

        if (isEq) {
            int itemWidth = 55;
            int itemHeight = juce::jmin(innerArea.getHeight() - 25, 250);
            for (int i = 0; i < sliders.size(); ++i) {
                labels[i]->setBounds(x, y, itemWidth, 18);
                sliders[i]->setBounds(x, y + 20, itemWidth, itemHeight);
                x += itemWidth + 8;
            }
        } else {
            int rowHeight = 35;
            int labelWidth = 120;
            int sliderWidth = innerArea.getWidth() - labelWidth - 10;

            // Choice params first
            for (int i = 0; i < comboBoxes.size(); ++i) {
                choiceLabels[i]->setBounds(x, y, labelWidth, rowHeight);
                comboBoxes[i]->setBounds(x + labelWidth + 10, y, juce::jmin(sliderWidth, 200), rowHeight);
                y += rowHeight + 8;
            }

            // Float params
            for (int i = 0; i < sliders.size(); ++i) {
                labels[i]->setBounds(x, y, labelWidth, rowHeight);
                sliders[i]->setBounds(x + labelWidth + 10, y, sliderWidth, rowHeight);
                y += rowHeight + 8;
            }
        }
    }

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenericModuleEditor)
};

} // namespace ui
} // namespace eeval
