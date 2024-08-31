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

juce::AudioBuffer<float> loadWavFile(const juce::File& file);
