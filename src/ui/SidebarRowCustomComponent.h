#pragma once
#include <JuceHeader.h>
#include "Theme.h"

namespace eeval {
namespace ui {

class SidebarRowCustomComponent : public juce::Component {
public:
    SidebarRowCustomComponent(juce::AudioProcessorValueTreeState& apvtsToUse, const std::string& mId, const juce::String& name, int r)
        : apvts(apvtsToUse), moduleId(mId), moduleName(name), rowNumber(r)
    {
        // Bypass icon/button
        bypassToggle.setClickingTogglesState(true);
        bypassToggle.setButtonText(""); // Using custom draw later
        addAndMakeVisible(bypassToggle);

        auto paramId = moduleId + ".bypass";
        bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, paramId, bypassToggle);
    }

    ~SidebarRowCustomComponent() override = default;

    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds();
        
        if (isSelected) {
            g.setColour(theme::bgOverlay);
            g.fillRoundedRectangle(area.toFloat(), 4.0f);
        }
        
        g.setColour(isSelected ? theme::textPrimary : theme::textSecondary);
        g.setFont(16.0f);
        
        // Push text inward to leave room for bypass button
        g.drawText("  " + moduleName, area.withTrimmedLeft(40), juce::Justification::centredLeft, true);

        // Draw simple bypass "LED" indicator over the toggle area
        auto btnArea = bypassToggle.getBounds();
        g.setColour(bypassToggle.getToggleState() ? theme::accentDanger : theme::textSecondary.withAlpha(0.3f));
        g.drawRoundedRectangle(btnArea.toFloat(), 4.0f, 2.0f);
        if (bypassToggle.getToggleState()) {
            g.fillRoundedRectangle(btnArea.reduced(3).toFloat(), 3.0f);
        }
    }

    void resized() override {
        auto area = getLocalBounds();
        bypassToggle.setBounds(area.removeFromLeft(30).reduced(5));
    }

    void update(bool selected) {
        isSelected = selected;
        repaint();
    }

    int getRowNumber() const { return rowNumber; }

private:
    juce::AudioProcessorValueTreeState& apvts;
    std::string moduleId;
    juce::String moduleName;
    int rowNumber;
    bool isSelected = false;

    juce::ToggleButton bypassToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SidebarRowCustomComponent)
};

} // namespace ui
} // namespace eeval
