/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
CrushOnYouAudioProcessorEditor::CrushOnYouAudioProcessorEditor (CrushOnYouAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (900, 500);

    // Setup your sliders and other gui components - - - -
    auto& params = processor.getParameters();
    AudioParameterFloat* audioParam;
    AudioParameterChoice* modeOfCrush = (AudioParameterChoice*)params.getUnchecked(3);

    // standard bitcrush button UI
    qlBt.setToggleable(true);
    qlBt.setToggleState(modeOfCrush->getIndex() == 0, true);
    qlBt.setButtonText("Standard");
    addAndMakeVisible(qlBt);
    qlBt.onClick = [this] { changeCrushMode(&qlBt); };

    // bitshift button UI
    shiftBt.setToggleable(true);
    qlBt.setToggleState(modeOfCrush->getIndex() == 1, true);
    shiftBt.setButtonText("Bitshift");
    addAndMakeVisible(shiftBt);
    shiftBt.onClick = [this] { changeCrushMode(&shiftBt); };

    // bitmask bit buttons UI
    for (int i = 0; i < 32; i++) {
        bitBts[i].setToggleable(true);
        bitBts[i].setToggleState(false, true);
        bitBts[i].setButtonText("0");
        addAndMakeVisible(bitBts[i]);
        bitBts[i].onClick = [this, i] { flipBit(i); };
    }

    maskLabel.setText("Bitmask (IEEE 754)", dontSendNotification);
    maskLabel.setJustificationType(Justification::centred);
    maskLabel.setSize(450, 30);
    maskLabel.setCentrePosition(450, 420);
    addAndMakeVisible(maskLabel);

    highLable.setText("31", dontSendNotification);
    highLable.setJustificationType(Justification::centred);
    highLable.setSize(50, 30);
    highLable.setCentrePosition(39, 425);
    addAndMakeVisible(highLable);

    lowLabel.setText("0", dontSendNotification);
    lowLabel.setJustificationType(Justification::centred);
    lowLabel.setSize(50, 30);
    lowLabel.setCentrePosition(861, 425);
    addAndMakeVisible(lowLabel);

    // Wet/Dry knob ui
    audioParam = (AudioParameterFloat*)params.getUnchecked(0);
    mixKnob.setRotaryParameters((5 * MathConstants<float>::pi) / 4, (11 * MathConstants<float>::pi) / 4, true);
    mixKnob.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mixKnob.setTextBoxStyle(Slider::NoTextBox, true, 70, 20);
    mixKnob.setRange(audioParam->range.start, audioParam->range.end);
    mixKnob.setValue(audioParam->get(), dontSendNotification);
    mixKnob.setDoubleClickReturnValue(true, 0.0f);
    mixKnob.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(mixKnob);
    mixKnob.addListener(this);

    mixLabel.attachToComponent(&mixKnob, false);
    mixLabel.setText("Mix", dontSendNotification);
    mixLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(mixLabel);

    // Decimate knob ui
    audioParam = (AudioParameterFloat*)params.getUnchecked(1);
    decimateKnob.setRotaryParameters((5 * MathConstants<float>::pi) / 4, (11 * MathConstants<float>::pi) / 4, true);
    decimateKnob.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    decimateKnob.setTextBoxStyle(Slider::TextBoxBelow, true, 70, 20);
    decimateKnob.setRange(audioParam->range.start, audioParam->range.end);
    decimateKnob.setValue(audioParam->get(), dontSendNotification);
    decimateKnob.setDoubleClickReturnValue(true, 0.0f);
    decimateKnob.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(decimateKnob);
    decimateKnob.addListener(this);

    decimateLabel.attachToComponent(&decimateKnob, false);
    decimateLabel.setText("Downsample Factor", dontSendNotification);
    decimateLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(decimateLabel);

    // Bitcrush knob ui
    audioParam = (AudioParameterFloat*)params.getUnchecked(2);
    depthKnob.setRotaryParameters((5 * MathConstants<float>::pi) / 4, (11 * MathConstants<float>::pi) / 4, true);
    depthKnob.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    depthKnob.setTextBoxStyle(Slider::TextBoxBelow, true, 70, 20);
    depthKnob.setRange(audioParam->range.start, audioParam->range.end);
    depthKnob.setValue(audioParam->get(), dontSendNotification);
    depthKnob.setDoubleClickReturnValue(true, 0.0f);
    depthKnob.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(depthKnob);
    depthKnob.addListener(this);

    depthLabel.attachToComponent(&depthKnob, false);
    depthLabel.setText("Bits", dontSendNotification);
    depthLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(depthLabel);
}

CrushOnYouAudioProcessorEditor::~CrushOnYouAudioProcessorEditor()
{
}

//==============================================================================
void CrushOnYouAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);
}

void CrushOnYouAudioProcessorEditor::resized()
{
    mainGrid.justifyItems = Grid::JustifyItems::center;
    mainGrid.alignItems = Grid::AlignItems::center;

    using Track = Grid::TrackInfo;
    using Fr = Grid::Fr;

    mainGrid.templateRows = { Track(Fr(1)), Track(Fr(3)), Track(Fr(1)) };
    mainGrid.templateColumns = 
    { 
        Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),
        Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),
        Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),
        Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),
        Track(Fr(1)), Track(Fr(1))
    };

    mainGrid.items.add(GridItem(qlBt)        .withArea(1, 13, 1, 17).withWidth(100.0f).withHeight(30.0f));
    mainGrid.items.add(GridItem(shiftBt)     .withArea(1, 19, 1, 23).withWidth(100.0f).withHeight(30.0f));

    mainGrid.items.add(GridItem(decimateKnob).withArea(2, 4, 2, 12).withWidth(175.0f).withHeight(175.0f));
    mainGrid.items.add(GridItem(depthKnob)   .withArea(2, 14, 2, 22).withWidth(175.0f).withHeight(175.0f));
    mainGrid.items.add(GridItem(mixKnob)     .withArea(2, 24, 2, 32).withWidth(125.0f).withHeight(125.0f));
    for (int i = 1; i <= 32; i++) {
        mainGrid.items.add(GridItem(bitBts[i-1]).withArea(3, 32-i+2, 3, 32-i+2).withWidth(25.0f).withHeight(25.0f));
    }

    mainGrid.performLayout(getLocalBounds());
}

void CrushOnYouAudioProcessorEditor::changeCrushMode(TextButton* pressed) {
    auto& params = processor.getParameters();

    AudioParameterChoice* choiceParam = (AudioParameterChoice*)params.getUnchecked(3); // Filter type is 3rd param
    if (pressed == &qlBt) {
        *choiceParam = 0; // 0th choice is ql;
        qlBt.setToggleState(true, false);
        shiftBt.setToggleState(false, false);
    }
    else {
        *choiceParam = 1;
        shiftBt.setToggleState(true, false);
        qlBt.setToggleState(false, false);
    }
}

void CrushOnYouAudioProcessorEditor::flipBit(int bitNumber) {
    auto& params = processor.getParameters();
    AudioParameterBool* boolParam = (AudioParameterBool*)params.getUnchecked(5 + bitNumber);

    bitBts[bitNumber].setToggleState(!bitBts[bitNumber].getToggleState(), false);
    *boolParam = bitBts[bitNumber].getToggleState();
    if (*boolParam)
        bitBts[bitNumber].setButtonText("1");
    else
        bitBts[bitNumber].setButtonText("0");
}

void CrushOnYouAudioProcessorEditor::sliderValueChanged(Slider* slider) {
    auto& params = processor.getParameters();

    // Check if slider is a Factor Slider
    if (&decimateKnob == slider) { // If slider has same memory address as filterFcSliders[i], they are the same slider
        AudioParameterInt* audioParam = (AudioParameterInt*)params.getUnchecked(1);
        *audioParam = decimateKnob.getValue();
    }
    // Check if slider is a Depth Slider
    if (&depthKnob == slider) {
        AudioParameterInt* audioParam = (AudioParameterInt*)params.getUnchecked(2);
        *audioParam = depthKnob.getValue();
    }
    // Check if slider is a Mix Slider
    if (&mixKnob == slider) {
        AudioParameterFloat* audioParam = (AudioParameterFloat*)params.getUnchecked(0);
        *audioParam = mixKnob.getValue();
    }
}

void CrushOnYouAudioProcessorEditor::timerCallback() {
    // Animated knobs and sliders for parameter automation
    DBG("Is this working");
    auto& params = processor.getParameters();
    AudioParameterInt* factor;
    AudioParameterInt* depth;
    AudioParameterFloat* mix;
    AudioParameterChoice* crushType;

    factor = (AudioParameterInt*)params.getUnchecked(1);
    decimateKnob.setValue(factor->get(), dontSendNotification);

    depth = (AudioParameterInt*)params.getUnchecked(2);
    depthKnob.setValue(depth->get(), dontSendNotification);

    mix = (AudioParameterFloat*)params.getUnchecked(0);
    mixKnob.setValue(mix->get(), dontSendNotification);

    AudioParameterBool* bit;
    bool paramVal;
    for (int i = 0; i < 31; i++) {
        bit = (AudioParameterBool*)params.getUnchecked(5+i);
        paramVal = bit->get();
        bitBts[i].setToggleState(paramVal, true);
        if (paramVal)
            bitBts[i].setButtonText("1");
    }
}