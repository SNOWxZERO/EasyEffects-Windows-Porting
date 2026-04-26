#include "EQPlotComponent.h"
#include "../PluginProcessor.h"
#include "../dsp/SpectrumAnalyzer.h"
#include "../dsp/EffectRegistry.h"
#include "../dsp/EqualizerModule.h"

namespace eeval {
namespace ui {

EQPlotComponent::EQPlotComponent(EasyEffectsAudioProcessor& p, juce::AudioProcessorValueTreeState& vts, int slotIndex)
    : audioProcessor(p), apvts(vts), slotIdx(slotIndex)
{
    prefix = eeval::EffectRegistry::slotPrefix(slotIdx) + ".eq.";
    
    sampleRate = audioProcessor.getSampleRate();
    startTimerHz(60); // 60 FPS for smooth spectrum
    
    // Listen to all 10 bands
    for (int i = 0; i < 10; ++i) {
        std::string b = "band" + std::to_string(i);
        apvts.addParameterListener(prefix + b + ".enabled", this);
        apvts.addParameterListener(prefix + b + ".type", this);
        apvts.addParameterListener(prefix + b + ".freq", this);
        apvts.addParameterListener(prefix + b + ".gain", this);
        apvts.addParameterListener(prefix + b + ".q", this);
    }

    magnitudes.resize(300); // 300 points for the curve
}

EQPlotComponent::~EQPlotComponent() {
    for (int i = 0; i < 10; ++i) {
        std::string b = "band" + std::to_string(i);
        apvts.removeParameterListener(prefix + b + ".enabled", this);
        apvts.removeParameterListener(prefix + b + ".type", this);
        apvts.removeParameterListener(prefix + b + ".freq", this);
        apvts.removeParameterListener(prefix + b + ".gain", this);
        apvts.removeParameterListener(prefix + b + ".q", this);
    }
}

void EQPlotComponent::parameterChanged(const juce::String&, float) {
    needsUpdate = true;
}

void EQPlotComponent::timerCallback() {
    auto& analyzer = audioProcessor.getSpectrumAnalyzer();
    analyzer.performAnalysis();
    
    updateSpectrumPath();
    repaint();
}

void EQPlotComponent::updateResponse() {
    if (!needsUpdate) return;
    
    int numPoints = (int)magnitudes.size();
    std::fill(magnitudes.begin(), magnitudes.end(), 1.0f);

    for (int i = 0; i < 10; ++i) {
        std::string b = "band" + std::to_string(i);
        bool enabled = apvts.getRawParameterValue(prefix + b + ".enabled")->load() > 0.5f;
        if (!enabled) continue;

        int type = (int)apvts.getRawParameterValue(prefix + b + ".type")->load();
        float freq = apvts.getRawParameterValue(prefix + b + ".freq")->load();
        float gain = apvts.getRawParameterValue(prefix + b + ".gain")->load();
        float q = apvts.getRawParameterValue(prefix + b + ".q")->load();

        juce::dsp::IIR::Coefficients<float>::Ptr coeffs;
        float g = juce::Decibels::decibelsToGain(gain);

        switch (type) {
            case 0: coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, freq, q, g); break;
            case 1: coeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, freq, q); break;
            case 2: coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, freq, q); break;
            case 3: coeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, freq, q, g); break;
            case 4: coeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, freq, q, g); break;
            case 5: coeffs = juce::dsp::IIR::Coefficients<float>::makeNotch(sampleRate, freq, q); break;
            default: break;
        }

        if (coeffs != nullptr) {
            for (int p = 0; p < numPoints; ++p) {
                float f = getFreqForX((float)p / (float)numPoints);
                magnitudes[p] *= (float)coeffs->getMagnitudeForFrequency(f, sampleRate);
            }
        }
    }

    // Create Path
    responsePath.clear();
    auto w = (float)getWidth();
    auto h = (float)getHeight();

    for (int p = 0; p < numPoints; ++p) {
        float x = (float)p / (float)numPoints * w;
        float y = getYForMag(magnitudes[p]);
        
        if (p == 0) responsePath.startNewSubPath(x, y);
        else responsePath.lineTo(x, y);
    }

    needsUpdate = false;
}

void EQPlotComponent::updateSpectrumPath() {
    spectrumPath.clear();
    auto& analyzer = audioProcessor.getSpectrumAnalyzer();
    auto& data = analyzer.getDisplayData();
    
    auto w = (float)getWidth();
    auto h = (float)getHeight();
    
    float binStep = (float)audioProcessor.getSampleRate() / (float)eeval::dsp::SpectrumAnalyzer::fftSize;

    // Start from left edge
    spectrumPath.startNewSubPath(0.0f, h);

    bool hasPoints = false;
    float lastY = h;

    for (int i = 1; i < (int)data.size(); ++i) {
        float freq = (float)i * binStep;
        if (freq > 20000.0f) break;

        float x = getXForFreq(juce::jmax(freq, 20.0f)) * w;
        float y = juce::jmap<float>(data[i], -100.0f, 0.0f, h, 0.0f);
        y = juce::jlimit(0.0f, h, y);

        if (!hasPoints) {
            // Extend the first data point to the left edge
            spectrumPath.lineTo(0.0f, y);
            spectrumPath.lineTo(x, y);
            hasPoints = true;
        } else {
            spectrumPath.lineTo(x, y);
        }
        lastY = y;
    }
    
    if (hasPoints) {
        spectrumPath.lineTo(w, lastY);
        spectrumPath.lineTo(w, h);
        spectrumPath.closeSubPath();
    }
}

void EQPlotComponent::paint(juce::Graphics& g) {
    updateResponse();

    auto w = (float)getWidth();
    auto h = (float)getHeight();

    // Background
    g.fillAll(juce::Colour(0xff1a1a1a));

    // Grid (Logarithmic)
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    float freqs[] = { 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    for (auto f : freqs) {
        float x = getXForFreq(f) * w;
        g.drawVerticalLine((int)x, 0.0f, h);
        g.drawText(f >= 1000 ? juce::String(f/1000, 1) + "k" : juce::String((int)f), 
                   (int)x + 2, (int)h - 15, 40, 15, juce::Justification::left);
    }

    // Horizontal Grid (dB)
    for (float db = -24.0f; db <= 24.0f; db += 6.0f) {
        float y = getYForMag(juce::Decibels::decibelsToGain(db));
        g.drawHorizontalLine((int)y, 0.0f, w);
        g.drawText(juce::String((int)db) + " dB", 2, (int)y - 12, 40, 12, juce::Justification::left);
    }

    // Spectrum Analyzer
    g.setColour(juce::Colours::cyan.withAlpha(0.15f));
    g.fillPath(spectrumPath);
    g.setColour(juce::Colours::cyan.withAlpha(0.3f));
    g.strokePath(spectrumPath, juce::PathStrokeType(1.0f));

    // Response Curve
    g.setColour(juce::Colours::cyan.withAlpha(0.8f));
    g.strokePath(responsePath, juce::PathStrokeType(2.5f));
}

void EQPlotComponent::resized() {
    needsUpdate = true;
}

float EQPlotComponent::getXForFreq(float freq) const {
    auto logF = std::log10(freq);
    auto logMin = std::log10(20.0);
    auto logMax = std::log10(20000.0);
    return (float)((logF - logMin) / (logMax - logMin));
}

float EQPlotComponent::getFreqForX(float x) const {
    auto logMin = std::log10(20.0);
    auto logMax = std::log10(20000.0);
    return std::pow(10.0f, (float)(logMin + x * (logMax - logMin)));
}

float EQPlotComponent::getYForMag(float mag) const {
    auto db = juce::Decibels::gainToDecibels(mag);
    auto h = (float)getHeight();
    // Range -24dB to +24dB
    return juce::jmap(db, -24.0f, 24.0f, h, 0.0f);
}

} // namespace ui
} // namespace eeval
