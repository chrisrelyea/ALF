/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ALFTools.h"
#include "BinaryData.h"

//==============================================================================
ALFAudioProcessor::ALFAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

ALFAudioProcessor::~ALFAudioProcessor()
{

}

//==============================================================================
const juce::String ALFAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ALFAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ALFAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ALFAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ALFAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ALFAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ALFAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ALFAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ALFAudioProcessor::getProgramName (int index)
{
    return {};
}

void ALFAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ALFAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    // Initialize the filter
    lowPassFilter.reset();
    lowPassFilter.prepare(spec);
    lowPassFilterPost.reset();
    lowPassFilterPost.prepare(spec);
    
    
    // Send sample rate to target rate display for new kHz calculation
    sampleRateForLabel = sampleRate;

    
    
    // Set up the vinyl noise
    currentVinylIndex = 0;
    auto memStream = std::make_unique<juce::MemoryInputStream>(BinaryData::vinyl_wav, BinaryData::vinyl_wavSize, false);
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(std::move(memStream)));
    juce::int64 totalSamples = reader->lengthInSamples;
    vinylBuffer.setSize(reader->numChannels, static_cast<int>(totalSamples));    
    reader->read(vinylBuffer.getArrayOfWritePointers(), vinylBuffer.getNumChannels(), 0, static_cast<int>(totalSamples));
    
    
    
    
    
}

void ALFAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ALFAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ALFAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // ONCE parameter is set up, access the bitcrush block size here
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float subblockSize = apvts.getRawParameterValue(blockSizeParamID.getParamID())->load();  // get downsampling factor
        int numSamples = buffer.getNumSamples();
        
        if (apvts.getRawParameterValue(lpPreParamID.getParamID())->load() && subblockSize > 1) applyFilter(channel, subblockSize, getSampleRate(), lowPassFilter, spec, buffer);
        
        if (subblockSize > 1) bitcrushSamples(buffer, subblockSize, channel);   // resample at lower rate

        
        
        
        
        
        // if bit depth control enabled, change bit depth to new target bit depth
        // options are 24, 16, 12, 8, 4 (bitDepthVal parameter values 0, 1, 2, 3, 4 respectively)

        if (apvts.getRawParameterValue(bitDepthOnParamID.getParamID())->load()) {  // if bit depth control enabled
            int paramVal = apvts.getRawParameterValue(bitDepthValParamID.getParamID())->load();
            changeBitDepth(buffer, channel, paramVal);
        }
        
        if (apvts.getRawParameterValue(lpPostParamID.getParamID())->load() && subblockSize > 1) applyFilter(channel, subblockSize, getSampleRate(), lowPassFilter, spec, buffer);
        
        
        // add vinyl noise
        float noiseModifier = apvts.getRawParameterValue(noiseLevelParamID.getParamID())->load();
        if (channel < vinylBuffer.getNumChannels())
        {
            auto* vinylData = vinylBuffer.getReadPointer(channel);
            auto* channelData = buffer.getWritePointer (channel);
            for (int i = 0; i < numSamples; i++) {
                if (currentVinylIndex >= vinylBuffer.getNumSamples())
                {
                currentVinylIndex = 0; // Loop the vinyl buffer
                }
                channelData[i] += (vinylData[currentVinylIndex] * noiseModifier);
                currentVinylIndex++;
            }
        }
        
        buffer.applyGain(apvts.getRawParameterValue(gainValParamID.getParamID())->load());
        

    }
}

//==============================================================================
bool ALFAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ALFAudioProcessor::createEditor()
{
    return new ALFAudioProcessorEditor (*this);
}

//==============================================================================
void ALFAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ALFAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ALFAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout ALFAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterBool>(lpPreParamID,"Pre LP Filter", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(lpPostParamID,"Post LP Filter", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(blockSizeParamID, "Downsampling Factor", 1.0, 20.0, 1.0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(noiseLevelParamID, "Noise Level", 0.0, 1.0, 0.0));
    layout.add(std::make_unique<juce::AudioParameterInt>(noiseTypeParamID, "Noise Type", 0, 1, 0));
    layout.add(std::make_unique<juce::AudioParameterBool>(bitDepthOnParamID, "Bit Depth Control", false));
    layout.add(std::make_unique<juce::AudioParameterInt>(bitDepthValParamID, "Target Bit Depth", 0, 4, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(gainValParamID, "Output Gain", 0.0, 2.0, 1.0));

    
    
    return layout;
}
