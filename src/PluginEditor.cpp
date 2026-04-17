#include "PluginProcessor.h"
#include "PluginEditor.h"

EasyEffectsAudioProcessorEditor::EasyEffectsAudioProcessorEditor(EasyEffectsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
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
}
