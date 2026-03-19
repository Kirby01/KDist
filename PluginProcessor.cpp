#include "PluginProcessor.h"
#include "PluginEditor.h"

KDistAudioProcessor::KDistAudioProcessor()
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

KDistAudioProcessor::~KDistAudioProcessor()
{
}

const juce::String KDistAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KDistAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool KDistAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool KDistAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double KDistAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KDistAudioProcessor::getNumPrograms()
{
    return 1;
}

int KDistAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KDistAudioProcessor::setCurrentProgram (int)
{
}

const juce::String KDistAudioProcessor::getProgramName (int)
{
    return {};
}

void KDistAudioProcessor::changeProgramName (int, const juce::String&)
{
}

void KDistAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);

    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = 1;

    lpfL.reset();
    lpfR.reset();
    lpfL.prepare (spec);
    lpfR.prepare (spec);

    a = 1.0f;
    b = 1.0f;

    updateFilter();
}

void KDistAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KDistAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
   #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
   #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

      #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
      #endif

    return true;
   #endif
}
#endif

void KDistAudioProcessor::updateFilter()
{
    auto cutoff = apvts.getRawParameterValue ("lpf")->load();
    auto q      = apvts.getRawParameterValue ("res")->load();

    cutoff = juce::jlimit (20.0f, (float) (currentSampleRate * 0.49), cutoff);
    q      = juce::jlimit (0.1f, 4.0f, q);

    auto coeffs = Coeffs::makeLowPass (currentSampleRate, cutoff, q);

    *lpfL.coefficients = *coeffs;
    *lpfR.coefficients = *coeffs;
}

void KDistAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateFilter();

    auto inputGain  = apvts.getRawParameterValue ("input")->load();
    auto outputGain = apvts.getRawParameterValue ("output")->load();
    auto drive      = apvts.getRawParameterValue ("drive")->load();

    auto* left  = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float inL = left[i] * inputGain;
        const float inR = right != nullptr ? right[i] * inputGain : inL;

        const float l = inL;
        const float r = inR;

        // exact core, no eps
        a = (1.0f - 0.5f) * (std::abs (std::abs (l) + b * 0.5f))
          + 0.5f * std::abs (std::pow (((l * 0.5f + r * 0.5f) * drive), 2.0f)) / a;

        b = (1.0f - 0.09f) * (a + std::abs (b - a))
          + 0.09f * (a / b);

        float coreL = (l / a) * outputGain;
        float coreR = (r / a) * outputGain;

        coreL = lpfL.processSample (coreL);
        coreR = lpfR.processSample (coreR);

        left[i] = coreL;
        if (right != nullptr)
            right[i] = coreR;
    }
}

bool KDistAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* KDistAudioProcessor::createEditor()
{
    return new KDistAudioProcessorEditor (*this);
}

void KDistAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    }
}

void KDistAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout KDistAudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<APF> (
        "input", "Input",
        juce::NormalisableRange<float> (0.0f, 4.0f, 0.01f), 1.0f));

    params.push_back (std::make_unique<APF> (
        "output", "Output",
        juce::NormalisableRange<float> (0.0f, 4.0f, 0.01f), 1.0f));

    params.push_back (std::make_unique<APF> (
        "drive", "Drive",
        juce::NormalisableRange<float> (1.0f, 200.0f, 1.0f), 60.0f));

    params.push_back (std::make_unique<APF> (
        "lpf", "LPF Hz",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.35f), 12000.0f));

    params.push_back (std::make_unique<APF> (
        "res", "Resonance",
        juce::NormalisableRange<float> (0.1f, 4.0f, 0.01f), 0.7f));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KDistAudioProcessor();
}