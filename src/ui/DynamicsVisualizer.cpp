#include "DynamicsVisualizer.h"
#include "Theme.h"

namespace eeval {
namespace ui {

DynamicsVisualizer::DynamicsVisualizer(juce::AudioProcessorValueTreeState& vts, const std::string& prefix, const std::string& type)
    : apvts(vts), paramPrefix(prefix), effectType(type)
{
    startTimerHz(30);
}

void DynamicsVisualizer::setCurrentLevels(float inputDb, float outputDb) {
    // Exponential smoothing for the ball
    const float alpha = 0.3f;
    lastInputDb = lastInputDb * (1.0f - alpha) + inputDb * alpha;
    lastOutputDb = lastOutputDb * (1.0f - alpha) + outputDb * alpha;
}

void DynamicsVisualizer::timerCallback() {
    // Check if parameters changed to redraw static curve
    bool changed = false;
    
    auto getVal = [&](const std::string& suffix) {
        if (auto* p = apvts.getRawParameterValue(paramPrefix + suffix))
            return p->load();
        return 0.0f;
    };

    float newThresh = getVal("threshold");
    float newRatio = getVal("ratio");
    float newKnee = getVal("knee");

    if (newThresh != threshold || newRatio != ratio || newKnee != knee) {
        threshold = newThresh;
        ratio = newRatio;
        knee = newKnee;
        changed = true;
    }

    if (changed) updateStaticCurve();
    repaint();
}

void DynamicsVisualizer::updateStaticCurve() {
    staticCurve.clear();
    auto w = (float)getWidth();
    auto h = (float)getHeight();

    auto dbToX = [&](float db) { return juce::jmap(db, -60.0f, 0.0f, 0.0f, w); };
    auto dbToY = [&](float db) { return juce::jmap(db, -60.0f, 0.0f, h, 0.0f); };

    staticCurve.startNewSubPath(dbToX(-60.0f), dbToY(-60.0f));

    for (float inDb = -60.0f; inDb <= 0.0f; inDb += 0.5f) {
        float outDb = inDb;
        
        if (effectType == "compressor") {
            if (inDb > threshold) {
                outDb = threshold + (inDb - threshold) / ratio;
            }
        } else if (effectType == "gate") {
            if (inDb < threshold) {
                outDb = -100.0f; // Simplified gate
            }
        } else if (effectType == "expander") {
             if (inDb < threshold) {
                outDb = threshold + (inDb - threshold) * ratio;
            }
        }

        staticCurve.lineTo(dbToX(inDb), dbToY(outDb));
    }
}

void DynamicsVisualizer::paint(juce::Graphics& g) {
    auto w = (float)getWidth();
    auto h = (float)getHeight();

    // Background
    g.fillAll(theme::bgSurface.withAlpha(0.5f));
    
    // Grid
    g.setColour(theme::textSecondary.withAlpha(0.1f));
    for (float db = -60.0f; db <= 0.0f; db += 10.0f) {
        float x = juce::jmap(db, -60.0f, 0.0f, 0.0f, w);
        float y = juce::jmap(db, -60.0f, 0.0f, h, 0.0f);
        g.drawVerticalLine((int)x, 0, h);
        g.drawHorizontalLine((int)y, 0, w);
    }

    // Static Curve
    g.setColour(theme::textSecondary);
    g.strokePath(staticCurve, juce::PathStrokeType(1.5f));

    // Live Ball
    if (lastInputDb > -60.0f) {
        float ballX = juce::jmap(juce::jlimit(-60.0f, 0.0f, lastInputDb), -60.0f, 0.0f, 0.0f, w);
        float ballY = juce::jmap(juce::jlimit(-60.0f, 0.0f, lastOutputDb), -60.0f, 0.0f, h, 0.0f);

        g.setColour(juce::Colours::cyan);
        g.fillEllipse(ballX - 4, ballY - 4, 8, 8);
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.drawEllipse(ballX - 5, ballY - 5, 10, 10, 1.0f);
    }
    
    // Labels
    g.setColour(theme::textSecondary);
    g.setFont(10.0f);
    g.drawText("Out (dB)", 2, 2, 50, 10, juce::Justification::left);
    g.drawText("In (dB)", (int)w - 50, (int)h - 12, 45, 10, juce::Justification::right);
}

void DynamicsVisualizer::resized() {
    updateStaticCurve();
}

} // namespace ui
} // namespace eeval
