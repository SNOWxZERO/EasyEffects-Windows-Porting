#pragma once
#include <JuceHeader.h>

namespace eeval {
namespace theme {

// Color Palette (GTK4 / Libadwaita Dark)
const juce::Colour bgBase        = juce::Colour(0xFF242424); // Main Background
const juce::Colour bgSurface     = juce::Colour(0xFF1E1E1E); // Sidebar/Footer
const juce::Colour bgOverlay     = juce::Colour(0xFF303030); // Hover/Active states
const juce::Colour bgCard        = juce::Colour(0xFF2D2D2D); // Card/panel background
const juce::Colour borderSubtle  = juce::Colour(0xFF404040); // Subtle borders

const juce::Colour textPrimary   = juce::Colour(0xFFFFFFFF);
const juce::Colour textSecondary = juce::Colour(0xFFA0A0A0);

const juce::Colour accentPrimary = juce::Colour(0xFF3584E4); // Libadwaita Blue
const juce::Colour accentSuccess = juce::Colour(0xFF2ED573); // GTK Green (Spectrum)
const juce::Colour accentDanger  = juce::Colour(0xFFE01B24); // GTK Red

class CustomLookAndFeel : public juce::LookAndFeel_V4 {
public:
    CustomLookAndFeel() {
        // Window
        setColour(juce::ResizableWindow::backgroundColourId, bgBase);

        // Labels - CRITICAL: must set text color so all labels are visible
        setColour(juce::Label::textColourId, textPrimary);
        setColour(juce::Label::backgroundColourId, juce::Colour(0x00000000));

        // TextButtons
        setColour(juce::TextButton::buttonColourId, bgOverlay);
        setColour(juce::TextButton::buttonOnColourId, accentPrimary);
        setColour(juce::TextButton::textColourOffId, textPrimary);
        setColour(juce::TextButton::textColourOnId, textPrimary);

        // ListBox
        setColour(juce::ListBox::backgroundColourId, bgSurface);
        setColour(juce::ListBox::textColourId, textPrimary);
        setColour(juce::ListBox::outlineColourId, juce::Colour(0x00000000));

        // Sliders
        setColour(juce::Slider::thumbColourId, accentPrimary);
        setColour(juce::Slider::trackColourId, accentPrimary);
        setColour(juce::Slider::backgroundColourId, juce::Colour(0xFF3A3A3A));
        setColour(juce::Slider::rotarySliderFillColourId, accentPrimary);
        setColour(juce::Slider::rotarySliderOutlineColourId, bgOverlay);
        setColour(juce::Slider::textBoxTextColourId, textPrimary);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF1A1A1A));
        setColour(juce::Slider::textBoxOutlineColourId, borderSubtle);
        setColour(juce::Slider::textBoxHighlightColourId, accentPrimary.withAlpha(0.3f));

        // ComboBox
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1A1A1A));
        setColour(juce::ComboBox::textColourId, textPrimary);
        setColour(juce::ComboBox::arrowColourId, textSecondary);
        setColour(juce::ComboBox::outlineColourId, borderSubtle);

        // PopupMenu
        setColour(juce::PopupMenu::backgroundColourId, bgCard);
        setColour(juce::PopupMenu::textColourId, textPrimary);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, accentPrimary);
        setColour(juce::PopupMenu::highlightedTextColourId, textPrimary);

        // ScrollBar
        setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xFF555555));
        setColour(juce::ScrollBar::trackColourId, juce::Colour(0x00000000));
    }

    // Custom flat linear slider drawing (GTK style)
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                          juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (style == juce::Slider::LinearHorizontal)
        {
            float trackY = (float)y + (float)height * 0.5f;
            float trackHeight = 6.0f;
            float trackTop = trackY - trackHeight * 0.5f;

            // Background track
            g.setColour(juce::Colour(0xFF3A3A3A));
            g.fillRoundedRectangle((float)x, trackTop, (float)width, trackHeight, 3.0f);

            // Filled portion
            float fillWidth = sliderPos - (float)x;
            if (fillWidth > 0.0f)
            {
                g.setColour(accentPrimary);
                g.fillRoundedRectangle((float)x, trackTop, fillWidth, trackHeight, 3.0f);
            }

            // Thumb circle
            float thumbSize = 16.0f;
            g.setColour(textPrimary);
            g.fillEllipse(sliderPos - thumbSize * 0.5f, trackY - thumbSize * 0.5f, thumbSize, thumbSize);
            g.setColour(accentPrimary);
            g.fillEllipse(sliderPos - thumbSize * 0.5f + 2.0f, trackY - thumbSize * 0.5f + 2.0f,
                          thumbSize - 4.0f, thumbSize - 4.0f);
        }
        else if (style == juce::Slider::LinearVertical)
        {
            float trackX = (float)x + (float)width * 0.5f;
            float trackWidth = 6.0f;
            float trackLeft = trackX - trackWidth * 0.5f;

            // Background track
            g.setColour(juce::Colour(0xFF3A3A3A));
            g.fillRoundedRectangle(trackLeft, (float)y, trackWidth, (float)height, 3.0f);

            // Filled portion (from bottom up)
            float fillHeight = (float)(y + height) - sliderPos;
            if (fillHeight > 0.0f)
            {
                g.setColour(accentPrimary);
                g.fillRoundedRectangle(trackLeft, sliderPos, trackWidth, fillHeight, 3.0f);
            }

            // Thumb circle
            float thumbSize = 14.0f;
            g.setColour(textPrimary);
            g.fillEllipse(trackX - thumbSize * 0.5f, sliderPos - thumbSize * 0.5f, thumbSize, thumbSize);
            g.setColour(accentPrimary);
            g.fillEllipse(trackX - thumbSize * 0.5f + 2.0f, sliderPos - thumbSize * 0.5f + 2.0f,
                          thumbSize - 4.0f, thumbSize - 4.0f);
        }
        else
        {
            // Fallback for other styles (IncDecButtons etc.)
            LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 0.0f, 1.0f, style, slider);
        }
    }

    // Custom flat button drawing
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        auto baseColour = backgroundColour;

        if (shouldDrawButtonAsDown)
            baseColour = baseColour.brighter(0.1f);
        else if (shouldDrawButtonAsHighlighted)
            baseColour = baseColour.brighter(0.05f);

        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, 6.0f);

        g.setColour(borderSubtle);
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
    }
};

} // namespace theme
} // namespace eeval
