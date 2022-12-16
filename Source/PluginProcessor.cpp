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
    addParameter(wetDryParam = new AudioParameterFloat("Mix", // parameterID,
        "Mix", // parameterName,
        -1.0f, // fully dry,
        1.0f, // fully wet,
        1.0f)); // fully wet by default

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

    addParameter(masksEnabledParam = new AudioParameterBool("masksEnabled",
        "Enable Masks",
        true));

    constexpr unsigned mask0{ 0b00000000000000000000000000000001 }; // bit 0
    constexpr unsigned mask1{ 0b00000000000000000000000000000010 }; // bit 1
    constexpr unsigned mask2{ 0b00000000000000000000000000000100 }; // bit 2
    constexpr unsigned mask3{ 0b00000000000000000000000000001000 }; // bit 3
    constexpr unsigned mask4{ 0b00000000000000000000000000010000 }; // bit 4
    constexpr unsigned mask5{ 0b00000000000000000000000000100000 }; // bit 5
    constexpr unsigned mask6{ 0b00000000000000000000000001000000 }; // bit 6
    constexpr unsigned mask7{ 0b00000000000000000000000010000000 }; // bit 7

    constexpr unsigned mask8{ 0b00000000000000000000000100000000 }; // bit 8
    constexpr unsigned mask9{ 0b00000000000000000000001000000000 }; // bit 9
    constexpr unsigned mask10{ 0b00000000000000000000010000000000 }; // bit 10
    constexpr unsigned mask11{ 0b00000000000000000000100000000000 }; // bit 11
    constexpr unsigned mask12{ 0b00000000000000000001000000000000 }; // bit 12
    constexpr unsigned mask13{ 0b00000000000000000010000000000000 }; // bit 13
    constexpr unsigned mask14{ 0b00000000000000000100000000000000 }; // bit 14
    constexpr unsigned mask15{ 0b00000000000000001000000000000000 }; // bit 15

    constexpr unsigned mask16{ 0b00000000000000010000000000000000 }; // bit 16
    constexpr unsigned mask17{ 0b00000000000000100000000000000000 }; // bit 17
    constexpr unsigned mask18{ 0b00000000000001000000000000000000 }; // bit 18
    constexpr unsigned mask19{ 0b00000000000010000000000000000000 }; // bit 19
    constexpr unsigned mask20{ 0b00000000000100000000000000000000 }; // bit 20
    constexpr unsigned mask21{ 0b00000000001000000000000000000000 }; // bit 21
    constexpr unsigned mask22{ 0b00000000010000000000000000000000 }; // bit 22
    constexpr unsigned mask23{ 0b00000000100000000000000000000000 }; // bit 23

    constexpr unsigned mask24{ 0b00000001000000000000000000000000 }; // bit 24
    constexpr unsigned mask25{ 0b00000010000000000000000000000000 }; // bit 25
    constexpr unsigned mask26{ 0b00000100000000000000000000000000 }; // bit 26
    constexpr unsigned mask27{ 0b00001000000000000000000000000000 }; // bit 27
    constexpr unsigned mask28{ 0b00010000000000000000000000000000 }; // bit 28
    constexpr unsigned mask29{ 0b00100000000000000000000000000000 }; // bit 29
    constexpr unsigned mask30{ 0b01000000000000000000000000000000 }; // bit 30
    constexpr unsigned mask31{ 0b10000000000000000000000000000000 }; // bit 31

    AudioParameterBool* temp;
    
    for (int i = 0; i < 32; i++) {
        addParameter(temp = new AudioParameterBool("mask" + std::to_string(i), std::to_string(i), false));
        bitMaskParams.push_back(temp);
        isMaskUsed.push_back(false);
    }

    masks.push_back(mask0);
    masks.push_back(mask1);
    masks.push_back(mask2);
    masks.push_back(mask3);
    masks.push_back(mask4);
    masks.push_back(mask5);
    masks.push_back(mask6);
    masks.push_back(mask7);
    masks.push_back(mask8);
    masks.push_back(mask9);
    masks.push_back(mask10);
    masks.push_back(mask11);
    masks.push_back(mask12);
    masks.push_back(mask13);
    masks.push_back(mask14);
    masks.push_back(mask15);
    masks.push_back(mask16);
    masks.push_back(mask17);
    masks.push_back(mask18);
    masks.push_back(mask19);
    masks.push_back(mask20);
    masks.push_back(mask21);
    masks.push_back(mask22);
    masks.push_back(mask23);
    masks.push_back(mask24);
    masks.push_back(mask25);
    masks.push_back(mask26);
    masks.push_back(mask27);
    masks.push_back(mask28);
    masks.push_back(mask29);
    masks.push_back(mask30);
    masks.push_back(mask31);

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
    setWetDryBalance(wetDryParam->get());
    dsFactor = dsFactorParam->get();
    bitDepth = bitDepthParam->get();
    masksEnabled = masksEnabledParam->get();
    if (masksEnabled) {
        for (int i = 0; i < 32; i++) {
            isMaskUsed[i] = bitMaskParams[i]->get();
        }
    }
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
    // only update if bitdepth has changed
    if (bitDepthMem != bitDepth){
        ql = 1.0f / (pow(2, bitDepth) - 1.0f);
        bitDepthMem = bitDepth;
    }
    sample = ql * ((int)(sample / ql));
}

void CrushOnYouAudioProcessor::bitcrushBitshiftStrategy(float& sample) {
    float temp = sample;

    // yeah
    int toShift = *(int*)&temp;
    toShift >>= 27 - bitDepth;
    toShift <<= 27 - bitDepth;
    temp = *(float*)&toShift;

    sample = temp;
}

void CrushOnYouAudioProcessor::bitmask(float& sample) {

    float temp = sample;
    int toMask = *(int*)&temp;
    for (int i = 0; i < 32; i++) {
        if (isMaskUsed[i]) {
            toMask &= ~masks[i]; // 0 out bit at mask index
        }
    }
    temp = *(float*)&toMask;
    sample = temp;
}

void CrushOnYouAudioProcessor::setWetDryBalance(float userIn) {
    userIn = (userIn + 1.0f) / 4.0f;
    wetGain = sin(MathConstants<float>::pi * userIn);
    dryGain = cos(MathConstants<float>::pi * userIn);
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

        if (masksEnabled) {
            bitmask(tempL);
            bitmask(tempR);
        }

        decimate(tempL, samp);
        decimate(tempR, samp);

        channelL[samp] = dryGain*channelL[samp] + wetGain*tempL;
        channelR[samp] = dryGain*channelR[samp] + wetGain*tempR;
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
