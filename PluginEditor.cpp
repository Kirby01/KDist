#include "PluginEditor.h"

KDistAudioProcessorEditor::KDistAudioProcessorEditor (KDistAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setupSlider (inputSlider,  inputLabel,  "Input");
    setupSlider (outputSlider, outputLabel, "Output");
    setupSlider (driveSlider,  driveLabel,  "Drive");
    setupSlider (lpfSlider,    lpfLabel,    "LPF Hz");
    setupSlider (resSlider,    resLabel,    "Resonance");

    inputAttachment  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "input",  inputSlider);
    outputAttachment = std::make_unique<SliderAttachment> (audioProcessor.apvts, "output", outputSlider);
    driveAttachment  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "drive",  driveSlider);
    lpfAttachment    = std::make_unique<SliderAttachment> (audioProcessor.apvts, "lpf",    lpfSlider);
    resAttachment    = std::make_unique<SliderAttachment> (audioProcessor.apvts, "res",    resSlider);

    setSize (540, 220);
}

KDistAudioProcessorEditor::~KDistAudioProcessorEditor()
{
}

void KDistAudioProcessorEditor::setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void KDistAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    g.setColour (juce::Colours::white);
    g.setFont (22.0f);
    g.drawFittedText ("KDist", 0, 10, getWidth(), 30, juce::Justification::centred, 1);

    g.setFont (13.0f);
    g.drawFittedText ("Harmonious Records", 0, 38, getWidth(), 20, juce::Justification::centred, 1);
}

void KDistAudioProcessorEditor::resized()
{
    const int yLabel = 70;
    const int ySlider = 90;
    const int w = 100;
    const int h = 110;
    const int gap = 5;
    const int startX = 12;

    inputLabel .setBounds (startX + 0 * (w + gap), yLabel, w, 20);
    outputLabel.setBounds (startX + 1 * (w + gap), yLabel, w, 20);
    driveLabel .setBounds (startX + 2 * (w + gap), yLabel, w, 20);
    lpfLabel   .setBounds (startX + 3 * (w + gap), yLabel, w, 20);
    resLabel   .setBounds (startX + 4 * (w + gap), yLabel, w, 20);

    inputSlider .setBounds (startX + 0 * (w + gap), ySlider, w, h);
    outputSlider.setBounds (startX + 1 * (w + gap), ySlider, w, h);
    driveSlider .setBounds (startX + 2 * (w + gap), ySlider, w, h);
    lpfSlider   .setBounds (startX + 3 * (w + gap), ySlider, w, h);
    resSlider   .setBounds (startX + 4 * (w + gap), ySlider, w, h);
}