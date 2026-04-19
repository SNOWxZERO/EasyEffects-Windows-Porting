#pragma once
#include <JuceHeader.h>
#include "Theme.h"

namespace eeval {
namespace ui {

class SidebarRowCustomComponent : public juce::Component {
public:
    SidebarRowCustomComponent(juce::AudioProcessorValueTreeState& apvtsToUse, const std::string& mId, const std::string& name, int r)
        : apvts(apvtsToUse), moduleId(mId), rowNumber(r)
    {
        // Resolve display name
        displayName = juce::String(name).trim();
        if (displayName.isEmpty())
            displayName = juce::String(mId);

        // Capitalize first letter
        if (displayName.isNotEmpty())
            displayName = displayName.substring(0, 1).toUpperCase() + displayName.substring(1);

        // Read initial bypass state
        auto* param = apvts.getRawParameterValue(moduleId + ".bypass");
        if (param != nullptr)
            isBypassed = param->load() > 0.5f;
    }

    ~SidebarRowCustomComponent() override = default;

    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds();

        // Selected highlight: blue left-edge bar (like original GTK app)
        if (isSelected) {
            // Blue accent bar on left edge
            g.setColour(theme::accentPrimary);
            g.fillRoundedRectangle(area.removeFromLeft(4).toFloat(), 2.0f);

            // Subtle highlight background
            g.setColour(theme::bgOverlay);
            g.fillRoundedRectangle(area.toFloat(), 4.0f);
        }

        // Bypass indicator circle (left side)
        auto bypassArea = area.removeFromLeft(36);
        {
            float cx = (float)bypassArea.getCentreX();
            float cy = (float)bypassArea.getCentreY();
            float radius = 5.0f;

            if (isBypassed) {
                // Red filled circle = bypassed
                g.setColour(theme::accentDanger);
                g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
            } else {
                // Green outline circle = active
                g.setColour(theme::accentSuccess);
                g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 1.5f);
            }
        }

        // Module name text
        g.setColour(isSelected ? theme::textPrimary : theme::textSecondary);
        g.setFont(14.0f);
        g.drawText(displayName, area.reduced(4, 0), juce::Justification::centredLeft, true);
    }

    void mouseDown(const juce::MouseEvent& e) override {
        // Check if click is in the bypass indicator area (left 36px)
        if (e.x < 36) {
            toggleBypass();
            return;
        }

        // Find the parent ListBox and trigger row selection
        // Custom components cover the entire ListBox row, so clicks never reach
        // the ListBox's own click handler. We must do this manually.
        auto* listBox = findParentComponentOfClass<juce::ListBox>();
        if (listBox != nullptr) {
            listBox->selectRow(rowNumber);
        }
    }

    void resized() override {}

    void update(bool selected) {
        isSelected = selected;

        // Re-read bypass state
        auto* param = apvts.getRawParameterValue(moduleId + ".bypass");
        if (param != nullptr)
            isBypassed = param->load() > 0.5f;

        repaint();
    }

    int getRowNumber() const { return rowNumber; }

private:
    void toggleBypass() {
        auto* param = apvts.getParameter(moduleId + ".bypass");
        if (param != nullptr) {
            float currentVal = param->getValue();
            param->setValueNotifyingHost(currentVal > 0.5f ? 0.0f : 1.0f);
            isBypassed = !isBypassed;
            repaint();
        }
    }

    juce::AudioProcessorValueTreeState& apvts;
    std::string moduleId;
    juce::String displayName;
    int rowNumber;
    bool isSelected = false;
    bool isBypassed = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SidebarRowCustomComponent)
};

} // namespace ui
} // namespace eeval
