#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "GenericModuleEditor.h"

namespace eeval {
namespace ui {

class ConvolverEditor : public juce::Component, public juce::Timer {
public:
    ConvolverEditor(EasyEffectsAudioProcessor& proc, int slotIdx);
    ~ConvolverEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    EasyEffectsAudioProcessor& processor;
    int slotIndex;

    juce::TextButton loadBtn{"Load IR File..."};
    juce::TextButton clearBtn{"Clear"};
    juce::Label fileLabel;
    
    std::unique_ptr<GenericModuleEditor> genericEditor;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConvolverEditor)
};

} // namespace ui
} // namespace eeval
