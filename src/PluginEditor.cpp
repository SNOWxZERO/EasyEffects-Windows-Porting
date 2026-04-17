#include "PluginProcessor.h"
#include "PluginEditor.h"

EasyEffectsAudioProcessorEditor::EasyEffectsAudioProcessorEditor(EasyEffectsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    gainSlider.setSliderStyle(juce::Slider::LinearVertical);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "gain", gainSlider);
        
    addAndMakeVisible(gainSlider);
    
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
    // Place the slider in the center of the window
    auto area = getLocalBounds();
    gainSlider.setBounds(area.removeFromBottom(200).withSizeKeepingCentre(100, 150));
}
