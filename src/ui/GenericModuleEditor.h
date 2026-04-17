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
        // Setup Bypass Toggle
        bypassToggle.setButtonText("Bypass");
        bypassToggle.setClickingTogglesState(true);
        addAndMakeVisible(bypassToggle);
        
        auto bypassParamId = moduleId + ".bypass";
        bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, bypassParamId, bypassToggle);

        // Standard Parameters
        auto params = ModuleDescriptors::getParametersForModule(moduleId);
        
        for (const auto& pdesc : params) {
            auto* slider = new juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
            slider->setTextValueSuffix(" " + pdesc.unit);
            
            auto* label = new juce::Label(pdesc.id + "_label", pdesc.label);
            label->setJustificationType(juce::Justification::centred);
            label->attachToComponent(slider, false);

            addAndMakeVisible(slider);
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
            label->setJustificationType(juce::Justification::centred);
            label->attachToComponent(combo, false);
            labels.add(label);

            filterTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                apvts, "filter.type", *combo);
        }
    }

    ~GenericModuleEditor() override = default;

    void paint(juce::Graphics& g) override {
        // Draw module title
        g.setColour(theme::textPrimary);
        g.setFont(juce::Font(22.0f, juce::Font::bold));
        g.drawText(moduleName, getLocalBounds().removeFromTop(40).withTrimmedLeft(10), juce::Justification::centredLeft);
    }

    void resized() override {
        auto area = getLocalBounds();
        
        auto header = area.removeFromTop(40);
        bypassToggle.setBounds(header.removeFromRight(100).reduced(5));

        // Fixed Grid Layout
        area.reduce(20, 20); // padding
        
        int itemWidth = 100;
        int itemHeight = 120;
        int maxPerRow = juce::jmax(1, area.getWidth() / itemWidth);
        
        int x = area.getX();
        int y = area.getY();
        
        int drawnCount = 0;

        // Place special combo box first if exists
        if (filterTypeCombo != nullptr) {
            filterTypeCombo->setBounds(x + 10, y + 20, itemWidth - 20, 24);
            x += itemWidth;
            drawnCount++;
        }

        // Place all standard sliders
        for (auto* slider : sliders) {
            if (drawnCount > 0 && drawnCount % maxPerRow == 0) {
                x = area.getX();
                y += itemHeight;
            }
            slider->setBounds(x + 5, y + 20, itemWidth - 10, itemHeight - 20);
            x += itemWidth;
            drawnCount++;
        }
    }

private:
    juce::AudioProcessorValueTreeState& apvts;
    std::string moduleId;
    std::string moduleName;
    
    juce::TextButton bypassToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::Label> labels;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> attachments;

    std::unique_ptr<juce::ComboBox> filterTypeCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenericModuleEditor)
};

} // namespace ui
} // namespace eeval
