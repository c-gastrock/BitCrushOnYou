/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
CrushOnYouAudioProcessor::CrushOnYouAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(dsFactorParam = new AudioParameterInt("dsFactor", // parameterID,
        "Downsample Factor", // parameterName,
        1, // minValue,
        16, // maxValue,
        1)); // defaultValue
    addParameter(bitDepthParam = new AudioParameterInt("bitDepth", // parameterID,
        "Bit-Depth", // parameterName,
        2, // minValue,
        24, // maxValue,
        24)); // defaultValue

    const StringArray choices = { "QL", "Bit-Shift"};
    auto attributes = AudioParameterChoiceAttributes().withLabel("selected");
    addParameter(crushMethodParam = new AudioParameterChoice("crushMethod", // parameterID,
        "Bitcrush Method", // parameterName,
        choices, // choice list,
        0, // default (QL equation),
        attributes));
}

CrushOnYouAudioProcessor::~CrushOnYouAudioProcessor()
{
}

//==============================================================================
const String CrushOnYouAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CrushOnYouAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CrushOnYouAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CrushOnYouAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CrushOnYouAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CrushOnYouAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CrushOnYouAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CrushOnYouAudioProcessor::setCurrentProgram (int index)
{
}

const String CrushOnYouAudioProcessor::getProgramName (int index)
{
    return {};
}

void CrushOnYouAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void CrushOnYouAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void CrushOnYouAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CrushOnYouAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CrushOnYouAudioProcessor::updateParameters() {
    dsFactor = dsFactorParam->get();
    bitDepth = bitDepthParam->get();
    crushMode = crushMethodParam->getIndex(); // using JUCE String gave weird results, so using int
}

void CrushOnYouAudioProcessor::decimate(float& sample, const int& index)
{
    if (index % dsFactor == 0) {
        dsSamp = sample;
    }
    sample = dsSamp;
}

void CrushOnYouAudioProcessor::bitcrush(float& sample) {
    switch (crushMode) {
        case 0: bitcrushNormalStrategy(sample);
            break;
        case 1: bitcrushBitshiftStrategy(sample);
            break;
    }
}

void CrushOnYouAudioProcessor::bitcrushNormalStrategy(float& sample) {
    float ql = 1 / (pow(2, bitDepth) - 1);
    sample = ql * ((int)(sample / ql));
}

void CrushOnYouAudioProcessor::bitcrushBitshiftStrategy(float& sample) {
    float temp = sample;

    // yeah
    long  i = *(long*)&temp;
    i >>= 27 - bitDepth;
    i <<= 27 - bitDepth;
    temp = *(float*)&i;

    sample = temp;
}

void CrushOnYouAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //=========================================================================

    updateParameters();

    auto* channelL = buffer.getWritePointer(0);
    auto* channelR = buffer.getWritePointer(1);

    for (int samp = 0; samp < buffer.getNumSamples(); samp++)
    {
        float tempL = channelL[samp], tempR = channelR[samp];

        bitcrush(tempL);
        bitcrush(tempR);

        decimate(tempL, samp);
        decimate(tempR, samp);

        channelL[samp] = tempL;
        channelR[samp] = tempR;
    }
}

//==============================================================================
bool CrushOnYouAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* CrushOnYouAudioProcessor::createEditor()
{
    return new CrushOnYouAudioProcessorEditor (*this);
}

//==============================================================================
void CrushOnYouAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CrushOnYouAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CrushOnYouAudioProcessor();
}
