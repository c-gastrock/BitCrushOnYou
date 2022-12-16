/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

using namespace juce;

//==============================================================================
/**
*/
class CrushOnYouAudioProcessorEditor  : public AudioProcessorEditor, public Slider::Listener, public Timer
{
public:
    CrushOnYouAudioProcessorEditor (CrushOnYouAudioProcessor&);
    ~CrushOnYouAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void sliderValueChanged(Slider* slider) override;
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CrushOnYouAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrushOnYouAudioProcessorEditor);

    Grid mainGrid;
    Slider decimateKnob;
    Slider depthKnob;
    Slider mixKnob;

    TextButton qlBt;
    TextButton shiftBt;

    TextButton bitBts[32];

    Label decimateLabel;
    Label depthLabel;
    Label mixLabel;
    Label maskLabel;
    Label highLable, lowLabel;

    void changeCrushMode(TextButton *pressed);
    void changeMaskMode();
    void flipBit(int bitNumber);
};
