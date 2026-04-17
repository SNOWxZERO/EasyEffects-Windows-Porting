#pragma once
#include <JuceHeader.h>

namespace eeval {
namespace theme {

// Color Palette
const juce::Colour bgBase        = juce::Colour::fromString("#1E1E2E");
const juce::Colour bgSurface     = juce::Colour::fromString("#313244");
const juce::Colour bgOverlay     = juce::Colour::fromString("#45475A");

const juce::Colour textPrimary   = juce::Colour::fromString("#CDD6F4");
const juce::Colour textSecondary = juce::Colour::fromString("#A6ADC8");

const juce::Colour accentPrimary = juce::Colour::fromString("#89B4FA"); // Vibrant blue
const juce::Colour accentSuccess = juce::Colour::fromString("#A6E3A1"); // Green (for bypass ON)
const juce::Colour accentDanger  = juce::Colour::fromString("#F38BA8"); // Red

class CustomLookAndFeel : public juce::LookAndFeel_V4 {
public:
    CustomLookAndFeel() {
        setColour(juce::ResizableWindow::backgroundColourId, bgBase);
        setColour(juce::Slider::thumbColourId, accentPrimary);
        setColour(juce::Slider::rotarySliderFillColourId, accentPrimary);
        setColour(juce::Slider::rotarySliderOutlineColourId, bgOverlay);
        setColour(juce::Slider::textBoxTextColourId, textPrimary);
        setColour(juce::Slider::textBoxBackgroundColourId, bgBase);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        
        setColour(juce::TextButton::buttonColourId, bgSurface);
        setColour(juce::TextButton::buttonOnColourId, accentPrimary);
        setColour(juce::TextButton::textColourOffId, textPrimary);
        
        setColour(juce::ListBox::backgroundColourId, bgBase);
        setColour(juce::ListBox::textColourId, textSecondary);
        setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    }

    // Modern flat rotary slider
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, const float rotaryStartAngle,
                          const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
        auto centreX = (float)x + (float)width  * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Draw track
        g.setColour(bgOverlay);
        g.drawEllipse(rx, ry, rw, rw, 4.0f);

        // Draw fill arc
        juce::Path p;
        auto arcRadius = radius;
        p.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);
        
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(p, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Draw thumb marker
        juce::Path thumb;
        auto pointerLength = radius * 0.8f;
        auto pointerThickness = 3.0f;
        thumb.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        thumb.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.fillPath(thumb);
    }
    
    juce::Font getLabelFont(juce::Label& label) override {
        juce::ignoreUnused(label);
        return juce::Font(14.0f, juce::Font::bold);
    }
    
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override {
        return juce::Font(juce::jmin(15.0f, (float)buttonHeight * 0.6f));
    }
};

} // namespace theme
} // namespace eeval
