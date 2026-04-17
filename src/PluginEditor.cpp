#include "PluginProcessor.h"
#include "PluginEditor.h"

EasyEffectsAudioProcessorEditor::EasyEffectsAudioProcessorEditor(EasyEffectsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(900, 600);
}

EasyEffectsAudioProcessorEditor::~EasyEffectsAudioProcessorEditor()
{
}

void EasyEffectsAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF1E1E2E)); // Dark background
    
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("EasyEffects Windows — DSP Engine (Phase 3B)", 
                     getLocalBounds().removeFromTop(50), juce::Justification::centred, 1);

    g.setFont(14.0f);
    g.setColour(juce::Colour(0xFFCDD6F4));
    
    auto area = getLocalBounds().reduced(20).withTrimmedTop(60);
    
    auto moduleNames = audioProcessor.getActiveEffectNames();
    g.drawText("Active DSP Chain:", area.removeFromTop(25), juce::Justification::left);
    
    for (int i = 0; i < (int)moduleNames.size(); ++i) {
        g.drawText(juce::String(i + 1) + ". " + moduleNames[i], 
                   area.removeFromTop(22).withTrimmedLeft(20), juce::Justification::left);
    }
    
    area.removeFromTop(20);
    g.drawText("UI will be rebuilt in Phase 4B after all DSP modules are stable.",
               area.removeFromTop(25), juce::Justification::left);
    g.drawText("Use Options > Audio Settings to configure I/O devices.",
               area.removeFromTop(25), juce::Justification::left);
}

void EasyEffectsAudioProcessorEditor::resized()
{
}
