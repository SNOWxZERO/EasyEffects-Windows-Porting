#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include <functional>

namespace eeval {
namespace ui {

class SidebarRowCustomComponent : public juce::Component {
public:
    SidebarRowCustomComponent(juce::AudioProcessorValueTreeState& apvtsToUse,
                              int slotIdx,
                              const std::string& typeId,
                              const std::string& name,
                              int rowNum,
                              std::function<void(int)> onRemove = nullptr,
                              std::function<void(int)> onMoveUp = nullptr,
                              std::function<void(int)> onMoveDown = nullptr)
        : apvts(apvtsToUse), slotIndex(slotIdx), effectTypeId(typeId),
          rowNumber(rowNum), displayName(name),
          removeCallback(onRemove), moveUpCallback(onMoveUp), moveDownCallback(onMoveDown)
    {
        // Read initial bypass state
        std::string prefix = "slot" + std::to_string(slotIndex);
        auto* param = apvts.getRawParameterValue(prefix + ".bypass");
        if (param != nullptr)
            isBypassed = param->load() > 0.5f;
    }

    ~SidebarRowCustomComponent() override = default;

    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds();

        // Selected: blue left-edge bar
        if (isSelected) {
            g.setColour(theme::accentPrimary);
            g.fillRoundedRectangle(area.removeFromLeft(4).toFloat(), 2.0f);
            g.setColour(theme::bgOverlay);
            g.fillRoundedRectangle(area.toFloat(), 4.0f);
        }

        // Bypass indicator (left)
        auto bypassArea = area.removeFromLeft(30);
        {
            float cx = (float)bypassArea.getCentreX();
            float cy = (float)bypassArea.getCentreY();
            float radius = 5.0f;
            if (isBypassed) {
                g.setColour(theme::accentDanger);
                g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
            } else {
                g.setColour(theme::accentSuccess);
                g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 1.5f);
            }
        }

        // Remove button area (right side)
        auto rightArea = area.removeFromRight(60);

        // Up/Down arrows (right of text)
        auto upArea = rightArea.removeFromLeft(20);
        auto downArea = rightArea.removeFromLeft(20);
        auto closeArea = rightArea;

        // Draw up arrow
        g.setColour(theme::textSecondary);
        g.setFont(14.0f);
        g.drawText(juce::String::charToString(0x25B2), upArea, juce::Justification::centred); // ▲
        g.drawText(juce::String::charToString(0x25BC), downArea, juce::Justification::centred); // ▼
        g.setColour(theme::accentDanger.withAlpha(0.7f));
        g.setFont(16.0f);
        g.drawText(juce::String::charToString(0x00D7), closeArea, juce::Justification::centred); // ×

        // Module name
        g.setColour(isSelected ? theme::textPrimary : theme::textSecondary);
        g.setFont(13.0f);
        g.drawText(displayName, area.reduced(4, 0), juce::Justification::centredLeft, true);
    }

    void mouseDown(const juce::MouseEvent& e) override {
        auto area = getLocalBounds();

        // Bypass area (left 30px)
        if (e.x < 30) {
            toggleBypass();
            return;
        }

        // Right 60px: up(20) + down(20) + close(20)
        int rightStart = area.getWidth() - 60;
        if (e.x >= rightStart) {
            int offset = e.x - rightStart;
            if (offset < 20) {
                // Up arrow
                if (moveUpCallback) moveUpCallback(slotIndex);
                return;
            } else if (offset < 40) {
                // Down arrow
                if (moveDownCallback) moveDownCallback(slotIndex);
                return;
            } else {
                // Close/remove
                if (removeCallback) removeCallback(slotIndex);
                return;
            }
        }

        // Click on name: select row
        auto* listBox = findParentComponentOfClass<juce::ListBox>();
        if (listBox != nullptr)
            listBox->selectRow(rowNumber);
    }

    void resized() override {}

    void update(bool selected) {
        isSelected = selected;
        std::string prefix = "slot" + std::to_string(slotIndex);
        auto* param = apvts.getRawParameterValue(prefix + ".bypass");
        if (param != nullptr)
            isBypassed = param->load() > 0.5f;
        repaint();
    }

    int getRowNumber() const { return rowNumber; }

private:
    void toggleBypass() {
        std::string prefix = "slot" + std::to_string(slotIndex);
        auto* param = apvts.getParameter(prefix + ".bypass");
        if (param != nullptr) {
            float currentVal = param->getValue();
            param->setValueNotifyingHost(currentVal > 0.5f ? 0.0f : 1.0f);
            isBypassed = !isBypassed;
            repaint();
        }
    }

    juce::AudioProcessorValueTreeState& apvts;
    int slotIndex;
    std::string effectTypeId;
    int rowNumber;
    juce::String displayName;
    bool isSelected = false;
    bool isBypassed = false;

    std::function<void(int)> removeCallback;
    std::function<void(int)> moveUpCallback;
    std::function<void(int)> moveDownCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SidebarRowCustomComponent)
};

} // namespace ui
} // namespace eeval
