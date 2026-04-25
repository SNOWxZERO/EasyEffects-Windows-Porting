#pragma once

#include <JuceHeader.h>

namespace eeval {
namespace dsp {

/**
 * SpectrumAnalyzer - Handles FFT analysis of audio data with lock-free buffers.
 * Designed for professional visualization with logarithmic mapping and smoothing.
 */
class SpectrumAnalyzer {
public:
    static constexpr int fftOrder = 10; // 1024 points
    static constexpr int fftSize = 1 << fftOrder;

    SpectrumAnalyzer() 
        : forwardFFT(fftOrder),
          window(fftSize, juce::dsp::WindowingFunction<float>::hann)
    {
        std::fill(fftData.begin(), fftData.end(), 0.0f);
        std::fill(displayData.begin(), displayData.end(), 0.0f);
    }

    /**
     * Pushes audio samples into the FIFO buffer.
     * Thread-safe for the audio thread (non-blocking).
     */
    void pushBlock(const float* data, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            fifo[fifoIndex++] = data[i];
            if (fifoIndex >= fftSize) {
                if (!nextFFTBlockReady) {
                    std::copy(fifo.begin(), fifo.end(), fftData.begin());
                    nextFFTBlockReady = true;
                }
                fifoIndex = 0;
            }
        }
    }

    /**
     * Performs FFT and updates the display data.
     * Should be called from the UI thread (timer callback).
     */
    void performAnalysis() {
        if (nextFFTBlockReady) {
            window.multiplyWithWindowingTable(fftData.data(), fftSize);
            forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());
            nextFFTBlockReady = false;

            // Update display data with exponential decay
            const float decayRate = 0.85f;
            for (int i = 0; i < fftSize / 2; ++i) {
                float val = juce::Decibels::gainToDecibels(fftData[i] / (float)fftSize);
                val = juce::jlimit(-100.0f, 0.0f, val);
                
                if (val > displayData[i])
                    displayData[i] = val;
                else
                    displayData[i] = displayData[i] * decayRate + val * (1.0f - decayRate);
            }
        }
    }

    const std::array<float, fftSize / 2>& getDisplayData() const { return displayData; }

private:
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData; // Double size for complex transform if needed
    std::array<float, fftSize / 2> displayData;
    
    int fifoIndex = 0;
    std::atomic<bool> nextFFTBlockReady { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};

} // namespace dsp
} // namespace eeval
