#include "VisualEqualizerEditor.h"
#include "../dsp/EffectRegistry.h"

namespace eeval {
namespace ui {

VisualEqualizerEditor::VisualEqualizerEditor(juce::AudioProcessorValueTreeState& vts, int slotIndex)
    : apvts(vts), slotIdx(slotIndex), plot(vts, slotIndex)
{
    addAndMakeVisible(plot);

    titleLabel.setText("Parametric Equalizer", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    for (int i = 0; i < 10; ++i) {
        auto node = std::make_unique<EQNodeComponent>(apvts, slotIdx, i, plot);
        addAndMakeVisible(*node);
        nodes.push_back(std::move(node));
    }

    addBandBtn.onClick = [this] { addNextBand(); };
    addAndMakeVisible(addBandBtn);
}

void VisualEqualizerEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff252525));
}

void VisualEqualizerEditor::resized() {
    auto area = getLocalBounds().reduced(10);
    
    auto header = area.removeFromTop(30);
    titleLabel.setBounds(header.removeFromLeft(200));
    addBandBtn.setBounds(header.removeFromRight(100));

    plot.setBounds(area);

    // Position nodes
    for (auto& node : nodes) {
        node->updatePosition();
    }
}

void VisualEqualizerEditor::addNextBand() {
    std::string prefix = eeval::EffectRegistry::slotPrefix(slotIdx) + ".eq.band";
    for (int i = 0; i < 10; ++i) {
        auto* p = apvts.getParameter(prefix + std::to_string(i) + ".enabled");
        if (p && p->getValue() < 0.5f) {
            p->setValueNotifyingHost(1.0f);
            nodes[i]->updatePosition();
            plot.repaint();
            break;
        }
    }
}

} // namespace ui
} // namespace eeval
