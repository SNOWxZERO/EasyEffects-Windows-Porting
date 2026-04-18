#pragma once
#include <JuceHeader.h>
#include "../dsp/LevelMeterModule.h"
#include "Theme.h"

namespace eeval {
namespace ui {

class LevelMeterEditor : public juce::Component, public juce::Timer {
public:
    LevelMeterEditor(LevelMeterModule* meterRef) : meter(meterRef) {
        startTimerHz(30); // 30 FPS UI repaints
    }
    
    ~LevelMeterEditor() override {
        stopTimer();
    }

    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds();
        
        g.setColour(theme::bgBase); // Very dark footer bg
        g.fillRect(area);

        area.reduce(10, 5); // padding

        // Simple text readout
        g.setColour(theme::textSecondary);
        g.setFont(14.0f);
        juce::String text = juce::String(cachedPeak, 1) + " dB";
        g.drawText(text, area.removeFromLeft(60), juce::Justification::centredLeft);

        auto leftMeter = area.removeFromTop(area.getHeight() / 2 - 2);
        area.removeFromTop(4); // spacing
        auto rightMeter = area; // remaining bottom half
        
        drawMeterBar(g, leftMeter, cachedRms, cachedPeak);
        drawMeterBar(g, rightMeter, cachedRms, cachedPeak); 
    }

    void timerCallback() override {
        if (meter != nullptr) {
            float newRms = meter->getRms();
            float newPeak = meter->getPeak();
            
            // Only repaint if visually changed
            if (std::abs(newRms - cachedRms) > 0.5f || std::abs(newPeak - cachedPeak) > 0.5f) {
                cachedRms = newRms;
                cachedPeak = newPeak;
                repaint();
            }
        }
    }

private:
    LevelMeterModule* meter = nullptr;
    float cachedRms = -100.0f;
    float cachedPeak = -100.0f;

    void drawMeterBar(juce::Graphics& g, juce::Rectangle<int> bounds, float rmsDb, float peakDb) {        
        auto meterArea = bounds;
        
        // Draw background
        g.setColour(theme::bgOverlay);
        g.fillRoundedRectangle(meterArea.toFloat(), 2.0f);
        
        // Calculate width mappings (ranging from -60dB to 0dB)
        auto normalizeDb = [](float db) { return juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 60.0f); };
        
        float rmsRatio = normalizeDb(rmsDb);
        float peakRatio = normalizeDb(peakDb);
        
        // Draw RMS filled bar
        auto rmsRect = meterArea.withWidth((int)(meterArea.getWidth() * rmsRatio));
        g.setColour(theme::accentPrimary); // original green style later
        g.fillRoundedRectangle(rmsRect.toFloat(), 2.0f);
        
        // Draw Peak line
        if (peakRatio > 0.0f) {
            int peakX = meterArea.getX() + (int)(meterArea.getWidth() * peakRatio);
            peakX = juce::jlimit(meterArea.getX(), meterArea.getRight() - 2, peakX);
            g.setColour(theme::accentDanger);
            g.fillRect(peakX, meterArea.getY(), 2, meterArea.getHeight());
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeterEditor)
};

} // namespace ui
} // namespace eeval
