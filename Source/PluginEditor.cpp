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
    
    downsampleLabel.setText("Downsample Factor", juce::NotificationType::dontSendNotification);
    downsampleLabel.setJustificationType(juce::Justification::horizontallyCentred);
    downsampleLabel.attachToComponent(&downsampleSlider, false);
    downsampleLabel.setFont(defaultFont);
    addAndMakeVisible(downsampleLabel);
    
    
    // Noise Level
    noiseLevelSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    noiseLevelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16);
    noiseLevelSlider.setBounds(0,0,70,86);
    addAndMakeVisible(noiseLevelSlider);
    
    noiseLevelLabel.setText("Vinyl Noise", juce::NotificationType::dontSendNotification);
    noiseLevelLabel.setJustificationType(juce::Justification::horizontallyCentred);
    noiseLevelLabel.attachToComponent(&noiseLevelSlider,false);
    noiseLevelLabel.setFont(defaultFont);
    addAndMakeVisible(noiseLevelLabel);
    
    // kHz Label
    targetRateLabel.setText("44.1kHz", juce::NotificationType::dontSendNotification);
    targetRateLabel.setBounds(0,0,120,30);
    targetRateLabelFont.setHeight(20);
    targetRateLabel.setFont(targetRateLabelFont);
    addAndMakeVisible(targetRateLabel);
    downsampleSlider.onValueChange = [this] { 
        
        // get new rate by dividing sample rate by the block size
        float newRate = (audioProcessor.sampleRateForLabel/audioProcessor.apvts.getRawParameterValue(blockSizeParamID.getParamID())->load() / 1000);
        
        // convert to string with 2 decimal places
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << newRate;
        std::string newRateString = oss.str();
        
        targetRateLabel.setText( newRateString + " kHz", juce::NotificationType::dontSendNotification);

    
        
    };
    
    
    
    
    
    setSize (500, 330);
}

ALFAudioProcessorEditor::~ALFAudioProcessorEditor()
{
}

//==============================================================================
void ALFAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto gradient = juce::ColourGradient(juce::Colours::black,0.0f,0.0f,juce::Colours::darkgrey,500.0f,330.0f,false);
    g.setColour(juce::Colours::grey);
    g.setGradientFill(gradient);
    g.fillAll();
    
    auto rect = getLocalBounds().withHeight(75);
    auto rectGradient = juce::ColourGradient(juce::Colours::steelblue, 0.0f,0.f, juce::Colours::fuchsia, 500.0f, 0.0f, false);
    g.setGradientFill(rectGradient);
    g.fillRect(rect);
    
    auto image = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);
    int destWidth = image.getWidth() / 4;
    int destHeight = image.getHeight() / 4;
    g.drawImage(image, getWidth() /2 - destWidth / 2, 0 , destWidth, destHeight, 0, 0, image.getWidth(), image.getHeight());
}

void ALFAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    downsampleSlider.setTopLeftPosition(125, 125);
    noiseLevelSlider.setTopLeftPosition(100, 200);
    targetRateLabel.setTopLeftPosition(15,125);
}
