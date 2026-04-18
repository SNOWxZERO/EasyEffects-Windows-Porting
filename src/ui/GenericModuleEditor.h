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
        // Bypass was removed here since it is now handled by the SidebarRowCustomComponent!

        // Standard Parameters
        auto params = ModuleDescriptors::getParametersForModule(moduleId);
        
        for (const auto& pdesc : params) {
            juce::Slider* slider = nullptr;
            
            if (moduleId == "eq") {
                // EQ uses vertical sliders like the GTK app
                slider = new juce::Slider(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
            } else {
                // Other parameters use number spinners layout
                slider = new juce::Slider(juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft);
            }
            
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
        // Just clear background, no need to draw title since it's obvious in the sidebar GTK layout
        juce::ignoreUnused(g);
    }

    void resized() override {
        auto area = getLocalBounds().reduced(20);
        
        int x = area.getX();
        int y = area.getY();
        
        if (filterTypeCombo != nullptr) {
            filterTypeCombo->setBounds(x + 120, y, 150, 30);
            labels[0]->setBounds(x, y, 100, 30);
            y += 40;
        }

        if (moduleId == "eq") {
            // Horizontal layout for EQ sliders
            int itemWidth = 60;
            int itemHeight = 300;
            for (int i = 0; i < sliders.size(); ++i) {
                if (labels[i]->getText() == "Mix") continue; // draw Mix somewhere else or ignore
                sliders[i]->setBounds(x, y + 20, itemWidth, itemHeight);
                labels[i]->setBounds(x, y, itemWidth, 20);
                x += itemWidth + 10;
            }
        } else {
            // Vertical layout for GTK-style spinners
            int itemHeight = 30;
            int currentLabelIdx = filterTypeCombo != nullptr ? 1 : 0;
            
            for (int i = 0; i < sliders.size(); ++i) {
                // Layout: [Label         ] [ Spinner ]
                labels[currentLabelIdx]->setBounds(x, y, 150, itemHeight);
                labels[currentLabelIdx]->setJustificationType(juce::Justification::centredLeft);
                sliders[i]->setBounds(x + 160, y, 120, itemHeight);
                
                y += itemHeight + 10;
                currentLabelIdx++;
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
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenericModuleEditor)
};

} // namespace ui
} // namespace eeval
