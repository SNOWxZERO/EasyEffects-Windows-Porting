#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "../dsp/SpectrumAnalyzer.h"
#include "Theme.h"

namespace eeval {
namespace ui {

/**
 * SpectrumAnalyzerEditor - A global bar-style spectrum visualizer.
 * Updated to use the professional lock-free SpectrumAnalyzer engine.
 */
class SpectrumAnalyzerEditor : public juce::Component, private juce::Timer {
public:
    SpectrumAnalyzerEditor(EasyEffectsAudioProcessor& p) 
        : processor(p)
    {
        startTimerHz(30);
    }

    ~SpectrumAnalyzerEditor() override = default;

    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds();
        
        g.setColour(theme::bgSurface); 
        g.fillRoundedRectangle(area.toFloat(), 6.0f);
        
        area.reduce(10, 10);
        float width = (float)area.getWidth();
        float height = (float)area.getHeight();
        float startY = (float)area.getBottom();

        g.setColour(juce::Colour::fromString("#FF2ED573")); // EasyEffects Green

        int numBars = 64; 
        float rectWidth = (width / (float)numBars) - 2.0f;
        if (rectWidth < 1.0f) rectWidth = 1.0f;

        auto& analyzer = processor.getSpectrumAnalyzer();
        auto& data = analyzer.getDisplayData();

        for (int i = 0; i < numBars; ++i) {
            // Logarithmic mapping for bars
            float linearParam = (float)i / (float)numBars;
            float skew = std::pow(linearParam, 2.0f); 
            int dataIdx = (int)(skew * (float)data.size());
            dataIdx = juce::jlimit(0, (int)data.size() - 1, dataIdx);

            float dB = data[dataIdx];
            float mappedLevel = juce::jmap<float>(dB, -100.0f, 0.0f, 0.0f, 1.0f);

            float barHeight = mappedLevel * height;
            barHeight = juce::jlimit(2.0f, height, barHeight);

            float x = area.getX() + i * (rectWidth + 2.0f);
            float y = startY - barHeight;

            g.fillRoundedRectangle(x, y, rectWidth, barHeight, 2.0f);
        }
        
        g.setColour(theme::textSecondary);
        g.setFont(12.0f);
        g.drawText("20 Hz", area.getX(), area.getBottom() - 15, 50, 15, juce::Justification::bottomLeft);
        g.drawText("20 kHz", area.getRight() - 50, area.getBottom() - 15, 50, 15, juce::Justification::bottomRight);
    }

    void timerCallback() override {
        processor.getSpectrumAnalyzer().performAnalysis();
        repaint();
    }

private:
    EasyEffectsAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerEditor)
};

} // namespace ui
} // namespace eeval
