/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

const juce::ParameterID lpPreParamID {"lppre", 1};

const juce::ParameterID lpPostParamID {"lppost", 1};

const juce::ParameterID blockSizeParamID {"blocksize", 1};

const juce::ParameterID noiseLevelParamID {"noise", 1};

const juce::ParameterID noiseTypeParamID {"noisetype", 1};

const juce::ParameterID bitDepthOnParamID {"bitdepthon", 1};

const juce::ParameterID bitDepthValParamID {"bitdepthval", 1};

//==============================================================================
/**
*/
class ALFAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ALFAudioProcessor();
    ~ALFAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    
    juce::AudioProcessorValueTreeState apvts {
        *this, nullptr, "Parameters", createParameterLayout()
    };
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    
    float blockSizeForTextBox;
    double sampleRateForLabel;
    
    

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ALFAudioProcessor)
    juce::dsp::IIR::Filter<float> lowPassFilter;
    juce::dsp::IIR::Filter<float> lowPassFilterPost;
    juce::dsp::ProcessSpec spec;
    
    
    
    int currentVinylIndex;          // keep track of index within vinyl data to loop over full vinyl buffer
    juce::AudioBuffer<float> vinylBuffer;           // store vinyl sample audio data
    
};
