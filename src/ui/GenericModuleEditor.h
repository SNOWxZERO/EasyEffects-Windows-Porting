#pragma once
#include <JuceHeader.h>
#include "ModuleDescriptors.h"
#include "Theme.h"

namespace eeval {
namespace ui {

class GenericModuleEditor : public juce::Component {
public:
    GenericModuleEditor(juce::AudioProcessorValueTreeState& stateToUse, const std::string& modId, const std::string& modName)
        : apvts(stateToUse), moduleId(modId), moduleName(modName)
    {
        // Standard Parameters
        auto params = ModuleDescriptors::getParametersForModule(moduleId);

        for (const auto& pdesc : params) {
            juce::Slider* slider = nullptr;

            if (moduleId == "eq") {
                // EQ uses vertical sliders like the GTK app
                slider = new juce::Slider(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
                slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 20);
            } else {
                // Other modules: horizontal slider with value readout on right
                slider = new juce::Slider(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
                slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 28);
            }

            slider->setTextValueSuffix(" " + pdesc.unit);
            slider->setColour(juce::Slider::textBoxOutlineColourId, theme::borderSubtle);
            slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF1A1A1A));

            auto* label = new juce::Label(pdesc.id + "_label", pdesc.label);
            label->setJustificationType(juce::Justification::centredLeft);
            label->setColour(juce::Label::textColourId, theme::textSecondary);

            addAndMakeVisible(slider);
            addAndMakeVisible(label);
            sliders.add(slider);
            labels.add(label);

            attachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, pdesc.id, *slider));
        }

        // Special exception for Choice parameters (e.g. Filter Type)
        if (moduleId == "filter") {
            auto* combo = new juce::ComboBox();
            combo->addItem("Highpass", 1);
            combo->addItem("Lowpass", 2);
            combo->setJustificationType(juce::Justification::centred);
            addAndMakeVisible(combo);
            filterTypeCombo.reset(combo);

            auto* label = new juce::Label("filter.type_label", "Type");
            label->setJustificationType(juce::Justification::centredLeft);
            label->setColour(juce::Label::textColourId, theme::textSecondary);
            addAndMakeVisible(label);
            filterTypeLabel.reset(label);

            filterTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                apvts, "filter.type", *combo);
        }
    }

    ~GenericModuleEditor() override = default;

    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds();

        // Module title
        auto titleArea = area.removeFromTop(40);
        g.setColour(theme::textPrimary);
        g.setFont(18.0f);
        g.drawText(moduleName, titleArea.reduced(20, 0), juce::Justification::centredLeft, true);

        // Draw "Controls" card background
        auto cardArea = area.reduced(15, 5);
        g.setColour(theme::bgCard);
        g.fillRoundedRectangle(cardArea.toFloat(), 8.0f);
        g.setColour(theme::borderSubtle);
        g.drawRoundedRectangle(cardArea.toFloat(), 8.0f, 1.0f);

        // "Controls" label at top of card
        g.setColour(theme::textPrimary);
        g.setFont(14.0f);
        g.drawText("Controls", cardArea.getX() + 15, cardArea.getY() + 8, 100, 20, juce::Justification::centredLeft);
    }

    void resized() override {
        auto area = getLocalBounds();
        area.removeFromTop(40); // title
        auto cardArea = area.reduced(15, 5);
        auto innerArea = cardArea.reduced(15, 35); // padding inside card, skip "Controls" label

        int x = innerArea.getX();
        int y = innerArea.getY();

        if (moduleId == "eq") {
            // Horizontal layout for EQ sliders
            int itemWidth = 55;
            int itemHeight = juce::jmin(innerArea.getHeight() - 25, 250);
            for (int i = 0; i < sliders.size(); ++i) {
                labels[i]->setBounds(x, y, itemWidth, 18);
                sliders[i]->setBounds(x, y + 20, itemWidth, itemHeight);
                x += itemWidth + 8;
            }
        } else {
            // Vertical list layout for standard modules (like GTK Controls panel)
            int rowHeight = 35;
            int labelWidth = 120;
            int sliderWidth = innerArea.getWidth() - labelWidth - 10;

            // Filter type combo first
            if (filterTypeCombo != nullptr && filterTypeLabel != nullptr) {
                filterTypeLabel->setBounds(x, y, labelWidth, rowHeight);
                filterTypeCombo->setBounds(x + labelWidth + 10, y, juce::jmin(sliderWidth, 200), rowHeight);
                y += rowHeight + 8;
            }

            for (int i = 0; i < sliders.size(); ++i) {
                labels[i]->setBounds(x, y, labelWidth, rowHeight);
                sliders[i]->setBounds(x + labelWidth + 10, y, sliderWidth, rowHeight);
                y += rowHeight + 8;
            }
        }
    }

private:
    juce::AudioProcessorValueTreeState& apvts;
    std::string moduleId;
    std::string moduleName;

    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::Label> labels;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> attachments;

    std::unique_ptr<juce::ComboBox> filterTypeCombo;
    std::unique_ptr<juce::Label> filterTypeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenericModuleEditor)
};

} // namespace ui
} // namespace eeval
