/*
  ==============================================================================

    ALFTools.h
    Created: 6 Jun 2024 2:30:09pm
    Author:  Chris Relyea

  ==============================================================================
*/

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <JuceHeader.h>


void bitcrushSamples(juce::AudioBuffer<float>& buffer, float blockSize, int channel);

void applyFilter(int channel, float subblockSize, double sampleRate, juce::dsp::IIR::Filter<float>& lowPassFilter, juce::dsp::ProcessSpec spec, juce::AudioBuffer<float>& buffer);

void changeBitDepth(juce::AudioBuffer<float>& buffer, int channel, int paramVal);
