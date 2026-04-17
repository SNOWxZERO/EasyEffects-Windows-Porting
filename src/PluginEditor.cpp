#include "PluginProcessor.h"
#include "PluginEditor.h"

EasyEffectsAudioProcessorEditor::EasyEffectsAudioProcessorEditor(EasyEffectsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Setup Sidebar
    loadedModules = audioProcessor.getActiveEffectNames();
    sidebarList.setModel(this);
    addAndMakeVisible(sidebarList);

    // Setup Gain Area
    gainSlider.setSliderStyle(juce::Slider::LinearVertical);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "gain", gainSlider);
    gainEditorArea.addAndMakeVisible(gainSlider);
    addChildComponent(gainEditorArea);

    // Setup Compressor Area
    compThreshSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compThreshSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    compThreshAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "comp_threshold", compThreshSlider);
    compEditorArea.addAndMakeVisible(compThreshSlider);

    compRatioSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compRatioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    compRatioAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "comp_ratio", compRatioSlider);
    compEditorArea.addAndMakeVisible(compRatioSlider);

    compAttackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compAttackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    compAttackAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "comp_attack", compAttackSlider);
    compEditorArea.addAndMakeVisible(compAttackSlider);

    compReleaseSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compReleaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    compReleaseAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "comp_release", compReleaseSlider);
    compEditorArea.addAndMakeVisible(compReleaseSlider);
    addChildComponent(compEditorArea);

    sidebarList.selectRow(audioProcessor.getSelectedEditorIndex());
    updateEditorView(audioProcessor.getSelectedEditorIndex());

    setSize(900, 600);
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
    
    // Top Bar (Presets) soon
    auto topBar = area.removeFromTop(40);
    
    // Sidebar
    sidebarList.setBounds(area.removeFromLeft(200));

    // Right Content Area
    auto contentArea = area;
    gainEditorArea.setBounds(contentArea);
    compEditorArea.setBounds(contentArea);

    // Lay out Gain Area components relative to its own bounds
    auto gainBounds = gainEditorArea.getLocalBounds();
    gainSlider.setBounds(gainBounds.withSizeKeepingCentre(100, 250));

    // Lay out Compressor Area components
    auto compBounds = compEditorArea.getLocalBounds();
    int knobWidth = compBounds.getWidth() / 4;
    compThreshSlider.setBounds(compBounds.removeFromLeft(knobWidth).withSizeKeepingCentre(knobWidth, knobWidth).reduced(10));
    compRatioSlider.setBounds(compBounds.removeFromLeft(knobWidth).withSizeKeepingCentre(knobWidth, knobWidth).reduced(10));
    compAttackSlider.setBounds(compBounds.removeFromLeft(knobWidth).withSizeKeepingCentre(knobWidth, knobWidth).reduced(10));
    compReleaseSlider.setBounds(compBounds.withSizeKeepingCentre(knobWidth, knobWidth).reduced(10));
}

int EasyEffectsAudioProcessorEditor::getNumRows()
{
    return (int)loadedModules.size();
}

void EasyEffectsAudioProcessorEditor::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::darkgrey);
    else
        g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    if (rowNumber < loadedModules.size())
        g.drawText(loadedModules[rowNumber], 10, 0, width - 20, height, juce::Justification::centredLeft, true);
}

void EasyEffectsAudioProcessorEditor::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    updateEditorView(row);
}

void EasyEffectsAudioProcessorEditor::updateEditorView(int row)
{
    audioProcessor.setSelectedEditorIndex(row);

    gainEditorArea.setVisible(false);
    compEditorArea.setVisible(false);

    if (row >= 0 && row < loadedModules.size()) {
        auto name = loadedModules[row];
        if (name == "Gain") gainEditorArea.setVisible(true);
        else if (name == "Compressor") compEditorArea.setVisible(true);
    }
}
