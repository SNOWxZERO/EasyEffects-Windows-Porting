#include "EQNodeComponent.h"
#include "EQPlotComponent.h"
#include "EQNodeSettingsPanel.h"
#include "../dsp/EffectRegistry.h"

namespace eeval {
namespace ui {

EQNodeComponent::EQNodeComponent(juce::AudioProcessorValueTreeState& vts, int slotIdx, int bandIndex, EQPlotComponent& plot)
    : apvts(vts), slotIndex(slotIdx), bandIdx(bandIndex), parentPlot(plot)
{
    prefix = eeval::EffectRegistry::slotPrefix(slotIndex) + ".eq.band" + std::to_string(bandIdx) + ".";
    
    freqParam = apvts.getRawParameterValue(prefix + "freq");
    gainParam = apvts.getRawParameterValue(prefix + "gain");
    enabledParam = apvts.getRawParameterValue(prefix + "enabled");

    setMouseCursor(juce::MouseCursor::NormalCursor);
    setSize(16, 16);
}

void EQNodeComponent::paint(juce::Graphics& g) {
    if (enabledParam && enabledParam->load() < 0.5f) return;

    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawEllipse(getLocalBounds().toFloat().reduced(0.5f), 1.0f);

    auto color = juce::Colour::fromHSV((float)bandIdx / 10.0f, 0.7f, 0.9f, 1.0f);
    g.setColour(color);
    g.fillEllipse(getLocalBounds().toFloat().reduced(3.0f));
    
    g.setColour(juce::Colours::white);
    g.setFont(10.0f);
    g.drawText(juce::String(bandIdx + 1), getLocalBounds(), juce::Justification::centred);
}

void EQNodeComponent::mouseDown(const juce::MouseEvent& e) {
    if (e.mods.isRightButtonDown()) {
        juce::PopupMenu menu;
        menu.addItem(1, "Peaking", true, (int)apvts.getRawParameterValue(prefix + "type")->load() == 0);
        menu.addItem(2, "Low Pass", true, (int)apvts.getRawParameterValue(prefix + "type")->load() == 1);
        menu.addItem(3, "High Pass", true, (int)apvts.getRawParameterValue(prefix + "type")->load() == 2);
        menu.addItem(4, "Low Shelf", true, (int)apvts.getRawParameterValue(prefix + "type")->load() == 3);
        menu.addItem(5, "High Shelf", true, (int)apvts.getRawParameterValue(prefix + "type")->load() == 4);
        menu.addItem(6, "Notch", true, (int)apvts.getRawParameterValue(prefix + "type")->load() == 5);
        menu.addSeparator();
        menu.addItem(10, "Delete Band");

        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result >= 1 && result <= 6) {
                if (auto* p = apvts.getParameter(prefix + "type"))
                    p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1((float)(result - 1)));
            } else if (result == 10) {
                if (auto* p = apvts.getParameter(prefix + "enabled"))
                    p->setValueNotifyingHost(0.0f);
            }
            parentPlot.repaint();
        });
        return;
    }

    if (e.getNumberOfClicks() > 1) {
        if (auto* p = apvts.getParameter(prefix + "gain"))
            p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(0.0f));
        return;
    }

    // Launch Inspector on Left Click
    if (e.mods.isLeftButtonDown()) {
        auto* panel = new EQNodeSettingsPanel(apvts, slotIndex, bandIdx);
        juce::CallOutBox::launchAsynchronously(std::unique_ptr<EQNodeSettingsPanel>(panel), getScreenBounds(), nullptr);
    }

    dragger.startDraggingComponent(this, e);
}

void EQNodeComponent::mouseDrag(const juce::MouseEvent& e) {
    dragger.dragComponent(this, e, nullptr);

    // Update parameters based on center position relative to parent plot
    auto center = getBounds().getCentre().toFloat();
    auto plotW = (float)parentPlot.getWidth();
    auto plotH = (float)parentPlot.getHeight();

    // Map X -> Freq (Logarithmic)
    float xPercent = juce::jlimit(0.0f, 1.0f, center.x / plotW);
    auto logMin = std::log10(20.0);
    auto logMax = std::log10(20000.0);
    float freq = std::pow(10.0f, (float)(logMin + xPercent * (logMax - logMin)));

    // Map Y -> Gain (-24 to +24 dB)
    float yPercent = juce::jlimit(0.0f, 1.0f, center.y / plotH);
    float gain = juce::jmap(yPercent, 1.0f, 0.0f, -24.0f, 24.0f);

    if (auto* pFreq = apvts.getParameter(prefix + "freq"))
        pFreq->setValueNotifyingHost(pFreq->getNormalisableRange().convertTo0to1(freq));
    
    if (auto* pGain = apvts.getParameter(prefix + "gain"))
        pGain->setValueNotifyingHost(pGain->getNormalisableRange().convertTo0to1(gain));
}

void EQNodeComponent::mouseUp(const juce::MouseEvent&) {
}

void EQNodeComponent::updatePosition() {
    if (!freqParam || !gainParam) return;

    // Freq -> X
    auto logF = std::log10(freqParam->load());
    auto logMin = std::log10(20.0);
    auto logMax = std::log10(20000.0);
    float xPercent = (float)((logF - logMin) / (logMax - logMin));

    // Gain -> Y
    float yPercent = juce::jmap(gainParam->load(), -24.0f, 24.0f, 1.0f, 0.0f);

    setCentreRelative(xPercent, yPercent);
    setVisible(enabledParam && enabledParam->load() > 0.5f);
}

} // namespace ui
} // namespace eeval
