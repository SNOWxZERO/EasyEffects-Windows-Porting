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
        
        g.setColour(theme::textPrimary);
        g.setFont(juce::Font(22.0f, juce::Font::bold));
        g.drawText("Level Meter", area.removeFromTop(40).withTrimmedLeft(10), juce::Justification::centredLeft);

        area.reduce(20, 20);
        
        auto leftMeter = area.removeFromTop(40);
        auto rightMeter = area.removeFromTop(40).withTrimmedTop(10);
        
        drawMeterBar(g, leftMeter, "L", cachedRms, cachedPeak);
        drawMeterBar(g, rightMeter, "R", cachedRms, cachedPeak); // Displaying same for now since meter is mono-averaged internally, but laid out for future stereo expansion.
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

    void drawMeterBar(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& label, float rmsDb, float peakDb) {
        g.setColour(theme::textSecondary);
        g.setFont(14.0f);
        g.drawText(label, bounds.removeFromLeft(20), juce::Justification::centredLeft);
        
        auto meterArea = bounds;
        
        // Draw background
        g.setColour(theme::bgOverlay);
        g.fillRoundedRectangle(meterArea.toFloat(), 4.0f);
        
        // Calculate width mappings (ranging from -60dB to 0dB)
        auto normalizeDb = [](float db) { return juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 60.0f); };
        
        float rmsRatio = normalizeDb(rmsDb);
        float peakRatio = normalizeDb(peakDb);
        
        // Draw RMS filled bar
        auto rmsRect = meterArea.withWidth((int)(meterArea.getWidth() * rmsRatio));
        g.setColour(theme::accentPrimary);
        g.fillRoundedRectangle(rmsRect.toFloat(), 4.0f);
        
        // Draw Peak line
        if (peakRatio > 0.0f) {
            int peakX = meterArea.getX() + (int)(meterArea.getWidth() * peakRatio);
            peakX = juce::jlimit(meterArea.getX(), meterArea.getRight() - 2, peakX);
            g.setColour(theme::accentDanger);
            g.fillRect(peakX, meterArea.getY(), 2, meterArea.getHeight());
        }
        
        // Draw current value
        g.setColour(theme::textPrimary);
        g.drawText(juce::String(peakDb, 1) + " dB", meterArea.withTrimmedRight(10), juce::Justification::centredRight);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeterEditor)
};

} // namespace ui
} // namespace eeval
