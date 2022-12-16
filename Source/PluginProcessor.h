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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrushOnYouAudioProcessor);

    // User param variables ========================================================

    AudioParameterFloat* wetDryParam;
    AudioParameterInt* dsFactorParam;
    AudioParameterInt* bitDepthParam;
    AudioParameterChoice* crushMethodParam;
    AudioParameterBool* masksEnabledParam;
    std::vector<AudioParameterBool*> bitMaskParams;

    // Private algo variables ======================================================

    float wetGain, dryGain;

    int dsFactor;
    float dsSamp; // current sample kept in decimation algorithm

    int crushMode;

    int bitDepth;
    int bitDepthMem = -1;
    float ql;

    bool masksEnabled = false;
    std::vector<unsigned> masks;
    std::vector<bool> isMaskUsed;

    // Algo functions ==============================================================

    void setWetDryBalance(float userIn);
    void decimate(float& sample, const int& index);
    void bitcrush(float& sample);

        // uses equation from Pirkle page 544
        void bitcrushNormalStrategy(float& sample);

        // bitshift based crush, uses a hacky pointer cast trick from the fast inverse sqrt algorithm.
        // https://en.wikipedia.org/wiki/Fast_inverse_square_root#Overview_of_the_code
        void bitcrushBitshiftStrategy(float& sample);

    // uses bitmasks to create general distortion.
    // I think it's cool even if it's not necessarily a bit-depth reduction
    void bitmask(float& sample);

    // Helpers
    void updateParameters();
};
