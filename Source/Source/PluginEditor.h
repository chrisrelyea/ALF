/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class ALFAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ALFAudioProcessorEditor (ALFAudioProcessor&);
    ~ALFAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ALFAudioProcessor& audioProcessor;
    
    // downsample
    juce::Slider downsampleSlider;
    juce::Label downsampleLabel;
    juce::AudioProcessorValueTreeState::SliderAttachment downsampleAttachment { audioProcessor.apvts, blockSizeParamID.getParamID(), downsampleSlider
    };
    
    
    // noise level
    juce::Slider noiseLevelSlider;
    juce::Label noiseLevelLabel;
    juce::AudioProcessorValueTreeState::SliderAttachment noiseLevelAttachment { audioProcessor.apvts, noiseLevelParamID.getParamID(), noiseLevelSlider};
    

    // pre lp filter toggle
    juce::ToggleButton preLpButton;
    juce::AudioProcessorValueTreeState::ButtonAttachment preLPAttachment {audioProcessor.apvts,
        lpPreParamID.getParamID(), preLpButton};
    juce::Label preLPLabel;
    
    // post lp filter toggle
    juce::ToggleButton postLpButton;
    juce::AudioProcessorValueTreeState::ButtonAttachment postLPAttachment {audioProcessor.apvts,
        lpPostParamID.getParamID(), postLpButton};
    juce::Label postLPLabel;
    
    
    // bit depth control toggle
    juce::ToggleButton bitDepthControlButton;
    juce::AudioProcessorValueTreeState::ButtonAttachment bitDepthControlAttachment {audioProcessor.apvts,
        bitDepthOnParamID.getParamID(), bitDepthControlButton};
    juce::Label bitDepthControlLabel;
    

    // bit depth slider or buttons
    juce::Slider bitDepthSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment bitDepthSliderAttachment {audioProcessor.apvts,
        bitDepthValParamID.getParamID(), bitDepthSlider};
    
    juce::Label targetBitDepthLabel;
    
    
    // output gain
    
    

    // kHz textbox
    juce::Label targetRateLabel;

    
    juce::Font targetRateLabelFont = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::Futura_ttc,BinaryData::Futura_ttcSize));
    juce::Font defaultFont = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::Futura_ttc,BinaryData::Futura_ttcSize));

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ALFAudioProcessorEditor)
    
};
