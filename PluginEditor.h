#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class KDistAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit KDistAudioProcessorEditor (KDistAudioProcessor&);
    ~KDistAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    KDistAudioProcessor& audioProcessor;

    juce::Slider inputSlider;
    juce::Slider outputSlider;
    juce::Slider driveSlider;
    juce::Slider lpfSlider;
    juce::Slider resSlider;

    juce::Label inputLabel;
    juce::Label outputLabel;
    juce::Label driveLabel;
    juce::Label lpfLabel;
    juce::Label resLabel;

    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<SliderAttachment> lpfAttachment;
    std::unique_ptr<SliderAttachment> resAttachment;

    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KDistAudioProcessorEditor)
};