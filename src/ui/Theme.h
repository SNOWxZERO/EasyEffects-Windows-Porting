#pragma once
#include <JuceHeader.h>

namespace eeval {
namespace theme {

// Color Palette (GTK4 / Libadwaita Dark)
const juce::Colour bgBase        = juce::Colour::fromString("#242424"); // Main Background
const juce::Colour bgSurface     = juce::Colour::fromString("#1E1E1E"); // Sidebar/Footer
const juce::Colour bgOverlay     = juce::Colour::fromString("#303030"); // Hover/Active states

const juce::Colour textPrimary   = juce::Colour::fromString("#FFFFFF");
const juce::Colour textSecondary = juce::Colour::fromString("#A0A0A0");

const juce::Colour accentPrimary = juce::Colour::fromString("#3584E4"); // Libadwaita Blue (Sliders)
const juce::Colour accentSuccess = juce::Colour::fromString("#2ED573"); // GTK Green (Spectrum)
const juce::Colour accentDanger  = juce::Colour::fromString("#E01B24"); // GTK Red (Bypass/Peaks)

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

        // Clean flat slider tracks for the EQ LinearVertical sliders
        setColour(juce::Slider::trackColourId, bgSurface);
        setColour(juce::Slider::backgroundColourId, bgOverlay);

        // Spinner / NumberBox styling
        setColour(juce::Slider::textBoxHighlightColourId, accentPrimary.withAlpha(0.3f));
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
