#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "Theme.h"

namespace eeval {
namespace ui {

class SpectrumAnalyzerEditor : public juce::Component, private juce::Timer {
public:
    SpectrumAnalyzerEditor(EasyEffectsAudioProcessor& p) 
        : processor(p), 
          forwardFFT(EasyEffectsAudioProcessor::fftOrder), 
          window(EasyEffectsAudioProcessor::fftSize, juce::dsp::WindowingFunction<float>::hann) 
    {
        startTimerHz(30);
    }

    ~SpectrumAnalyzerEditor() override = default;

    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds();
        
        g.setColour(theme::bgSurface); // GTK Dark base
        g.fillRoundedRectangle(area.toFloat(), 6.0f);
        
        area.reduce(10, 10);
        float width = (float)area.getWidth();
        float height = (float)area.getHeight();
        float startY = (float)area.getBottom();

        g.setColour(juce::Colour::fromString("#FF2ED573")); // EasyEffects Green

        int numBins = 64; // Display 64 graphical bars across the screen
        float rectWidth = (width / (float)numBins) - 2.0f; // 2px gap between bars
        if (rectWidth < 1.0f) rectWidth = 1.0f;

        for (int i = 0; i < numBins; ++i) {
            // Logarithmic mapping for frequencies
            float linearParam = (float)i / (float)numBins;
            float skew = std::pow(linearParam, 2.0f); 
            int fftDataIndex = (int)(skew * (EasyEffectsAudioProcessor::fftSize / 2.0f));
            fftDataIndex = juce::jlimit(0, EasyEffectsAudioProcessor::fftSize / 2, fftDataIndex);

            // Fetch smoothed amplitude
            float level = smoothedLevels[i];

            // Normalize level height
            float barHeight = level * height;
            barHeight = juce::jlimit(2.0f, height, barHeight);

            float x = area.getX() + i * (rectWidth + 2.0f);
            float y = startY - barHeight;

            g.fillRoundedRectangle(x, y, rectWidth, barHeight, 2.0f);
        }
        
        // Draw bottom label axis
        g.setColour(theme::textSecondary);
        g.setFont(12.0f);
        g.drawText("20 Hz", area.getX(), area.getBottom() - 15, 50, 15, juce::Justification::bottomLeft);
        g.drawText("20 kHz", area.getRight() - 50, area.getBottom() - 15, 50, 15, juce::Justification::bottomRight);
    }

    void timerCallback() override {
        if (processor.nextFFTBlockReady) {
            // Perform windowing
            window.multiplyWithWindowingTable(processor.fftData.data(), EasyEffectsAudioProcessor::fftSize);
            
            // Forward transform to get frequency bins
            forwardFFT.performFrequencyOnlyForwardTransform(processor.fftData.data());

            // Smooth visuals manually for 64 bins
            int numBins = 64;
            for (int i = 0; i < numBins; ++i) {
                float linearParam = (float)i / (float)numBins;
                float skew = std::pow(linearParam, 2.0f); 
                int fftDataIndex = (int)(skew * (EasyEffectsAudioProcessor::fftSize / 2.0f));
                fftDataIndex = juce::jlimit(0, EasyEffectsAudioProcessor::fftSize / 2 - 1, fftDataIndex);
                
                // Map amplitude (Db scale approximation to [0,1])
                float level = juce::Decibels::gainToDecibels(processor.fftData[(size_t)fftDataIndex]) / 2.0f; 
                float mappedLevel = juce::jmap(level, -100.0f, 0.0f, 0.0f, 1.0f);
                mappedLevel = juce::jlimit(0.0f, 1.0f, mappedLevel);

                // Smooth Attack/Release for green visuals
                if (mappedLevel > smoothedLevels[i]) {
                    smoothedLevels[i] = juce::jlimit(0.0f, 1.0f, smoothedLevels[i] + 0.1f); // Fast attack
                } else {
                    smoothedLevels[i] = juce::jlimit(0.0f, 1.0f, smoothedLevels[i] - 0.05f); // Slow release
                }
            }

            processor.nextFFTBlockReady = false;
            repaint();
        }
    }

private:
    EasyEffectsAudioProcessor& processor;
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    
    std::array<float, 64> smoothedLevels = {0.0f};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerEditor)
};

} // namespace ui
} // namespace eeval
