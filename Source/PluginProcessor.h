/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

using namespace juce;

//==============================================================================
/**
*/
class CrushOnYouAudioProcessor  : public AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    CrushOnYouAudioProcessor();
    ~CrushOnYouAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrushOnYouAudioProcessor)

    // User param variables ========================================================

    AudioParameterInt* dsFactorParam;
    AudioParameterInt* bitDepthParam;
    AudioParameterChoice* crushMethodParam;

    // Private algo variables ======================================================

    int dsFactor;
    float dsSamp; // current sample kept in decimation algorithm

    int bitDepth;
    int crushMode;

    // Algo functions ==============================================================

    void decimate(float& sample, const int& index);
    void bitcrush(float& sample);

        // uses equation from Pirkle page 544
        void bitcrushNormalStrategy(float& sample);

        // bitshift based crush, uses a hacky pointer cast trick from the fast inverse sqrt algorithm.
        // https://en.wikipedia.org/wiki/Fast_inverse_square_root#Overview_of_the_code
        void bitcrushBitshiftStrategy(float& sample);

    // Helpers
    void updateParameters();
};
