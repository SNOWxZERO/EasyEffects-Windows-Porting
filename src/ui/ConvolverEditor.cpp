#include "ConvolverEditor.h"
#include "../dsp/ConvolverModule.h"

namespace eeval {
namespace ui {

ConvolverEditor::ConvolverEditor(EasyEffectsAudioProcessor& proc, int slotIdx)
    : processor(proc), slotIndex(slotIdx)
{
    // Embed the generic editor for the Mix/Presets UI
    genericEditor = std::make_unique<GenericModuleEditor>(
        processor.parameters, slotIndex, "convolver", "Convolver");
    addAndMakeVisible(genericEditor.get());

    addAndMakeVisible(loadBtn);
    addAndMakeVisible(clearBtn);
    
    fileLabel.setFont(juce::Font(14.0f));
    fileLabel.setColour(juce::Label::textColourId, eeval::theme::textPrimary);
    fileLabel.setText("No IR Loaded", juce::dontSendNotification);
    addAndMakeVisible(fileLabel);

    loadBtn.onClick = [this]() {
        fileChooser = std::make_unique<juce::FileChooser>("Select an Impulse Response (.wav)", juce::File{}, "*.wav");
        
        auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file.existsAsFile()) {
                if (auto* mod = processor.getChain().getSlotModule(slotIndex)) {
                    if (auto* conv = dynamic_cast<ConvolverModule*>(mod)) {
                        conv->loadImpulseResponse(file);
                    }
                }
            }
        });
    };

    clearBtn.onClick = [this]() {
        if (auto* mod = processor.getChain().getSlotModule(slotIndex)) {
            if (auto* conv = dynamic_cast<ConvolverModule*>(mod)) {
                conv->loadImpulseResponse(juce::File()); // empty file clears it
            }
        }
    };

    startTimerHz(10);
}

void ConvolverEditor::timerCallback() {
    // Update label with loaded IR name
    if (auto* mod = processor.getChain().getSlotModule(slotIndex)) {
        if (auto* conv = dynamic_cast<ConvolverModule*>(mod)) {
            std::string name = conv->getLoadedIRName();
            if (name.empty()) {
                fileLabel.setText("No IR Loaded", juce::dontSendNotification);
            } else {
                fileLabel.setText("Loaded: " + juce::String(name), juce::dontSendNotification);
            }
        }
    }
}

void ConvolverEditor::paint(juce::Graphics& g) {
    g.fillAll(eeval::theme::bgBase);
    
    // Draw a nice background panel for the IR loader
    auto r = getLocalBounds().removeFromTop(100).reduced(10);
    g.setColour(eeval::theme::bgSurface);
    g.fillRoundedRectangle(r.toFloat(), 6.0f);
    
    g.setColour(eeval::theme::borderSubtle);
    g.drawRoundedRectangle(r.toFloat(), 6.0f, 1.0f);
}

void ConvolverEditor::resized() {
    auto area = getLocalBounds();
    
    auto irPanel = area.removeFromTop(100).reduced(20);
    auto topRow = irPanel.removeFromTop(30);
    
    loadBtn.setBounds(topRow.removeFromLeft(120));
    topRow.removeFromLeft(10);
    clearBtn.setBounds(topRow.removeFromLeft(80));
    
    irPanel.removeFromTop(10);
    fileLabel.setBounds(irPanel);

    if (genericEditor) {
        genericEditor->setBounds(area);
    }
}

} // namespace ui
} // namespace eeval
