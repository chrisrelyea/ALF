/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ALFAudioProcessorEditor::ALFAudioProcessorEditor (ALFAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    // Downsample
    downsampleSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    downsampleSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    downsampleSlider.setBounds(0,0,250,40);
    addAndMakeVisible(downsampleSlider);
    
    downsampleLabel.setText("Downsample", juce::NotificationType::dontSendNotification);
    downsampleLabel.setJustificationType(juce::Justification::horizontallyCentred);
    downsampleLabel.attachToComponent(&downsampleSlider, false);
    addAndMakeVisible(downsampleLabel);
    
    
    // Noise Level
    noiseLevelSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    noiseLevelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16);
    noiseLevelSlider.setBounds(0,0,70,86);
    addAndMakeVisible(noiseLevelSlider);
    
    noiseLevelLabel.setText("Noise Level", juce::NotificationType::dontSendNotification);
    noiseLevelLabel.setJustificationType(juce::Justification::horizontallyCentred);
    noiseLevelLabel.attachToComponent(&noiseLevelSlider,false);
    addAndMakeVisible(noiseLevelLabel);
    
    
    
    
    
    
    setSize (500, 330);
}

ALFAudioProcessorEditor::~ALFAudioProcessorEditor()
{
}

//==============================================================================
void ALFAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void ALFAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    downsampleSlider.setTopLeftPosition(125, 125);
    noiseLevelSlider.setTopLeftPosition(100, 200);
}
