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
class CrushOnYouAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    CrushOnYouAudioProcessorEditor (CrushOnYouAudioProcessor&);
    ~CrushOnYouAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CrushOnYouAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrushOnYouAudioProcessorEditor)
};
