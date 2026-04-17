#include "PluginProcessor.h"
#include "PluginEditor.h"

EasyEffectsAudioProcessorEditor::EasyEffectsAudioProcessorEditor(EasyEffectsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    gainSlider.setSliderStyle(juce::Slider::LinearVertical);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "gain", gainSlider);
    addAndMakeVisible(gainSlider);

    // Setup Compressor Sliders
    compThreshSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compThreshSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    compThreshAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "comp_threshold", compThreshSlider);
    addAndMakeVisible(compThreshSlider);

    compRatioSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compRatioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    compRatioAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "comp_ratio", compRatioSlider);
    addAndMakeVisible(compRatioSlider);

    compAttackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compAttackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    compAttackAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "comp_attack", compAttackSlider);
    addAndMakeVisible(compAttackSlider);

    compReleaseSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compReleaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    compReleaseAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "comp_release", compReleaseSlider);
    addAndMakeVisible(compReleaseSlider);

    
    setSize(800, 600);
}

EasyEffectsAudioProcessorEditor::~EasyEffectsAudioProcessorEditor()
{
}

void EasyEffectsAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Phase 4: We will properly layout our DSP UI modules here
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    g.setColour(juce::Colours::white);
    g.setFont(30.0f);
    g.drawFittedText("EasyEffects Phase 0 - Development Build", getLocalBounds(), juce::Justification::centred, 1);
}

void EasyEffectsAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // Gain fits on the right
    gainSlider.setBounds(area.removeFromRight(100).withSizeKeepingCentre(100, 250));
    
    // Compressor knobs inside area
    auto topArea = area.removeFromTop(200);
    int knobWidth = topArea.getWidth() / 4;
    
    compThreshSlider.setBounds(topArea.removeFromLeft(knobWidth).reduced(10));
    compRatioSlider.setBounds(topArea.removeFromLeft(knobWidth).reduced(10));
    compAttackSlider.setBounds(topArea.removeFromLeft(knobWidth).reduced(10));
    compReleaseSlider.setBounds(topArea.reduced(10));
}
