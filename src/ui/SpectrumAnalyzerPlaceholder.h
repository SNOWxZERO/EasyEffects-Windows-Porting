#pragma once
#include <JuceHeader.h>
#include "Theme.h"

namespace eeval {
namespace ui {

class SpectrumAnalyzerPlaceholder : public juce::Component {
public:
    SpectrumAnalyzerPlaceholder() {}
    ~SpectrumAnalyzerPlaceholder() override = default;

    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds();
        g.setColour(theme::bgSurface);
        g.fillRoundedRectangle(area.toFloat(), 4.0f);
        
        g.setColour(theme::textSecondary);
        g.drawText("FFT Spectrum Analyzer (Placeholder)", area, juce::Justification::centred);
    }
};

} // namespace ui
} // namespace eeval
