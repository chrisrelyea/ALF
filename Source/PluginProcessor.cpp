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
    lowPassFilterPre.reset();
    lowPassFilterPre.prepare(spec);
    lowPassFilterPost.reset();
    lowPassFilterPost.prepare(spec);

    
    
    currentVinylIndex = 0;
    
    auto memStream = std::make_unique<juce::MemoryInputStream>(BinaryData::vinyl_wav, BinaryData::vinyl_wavSize, false);
    
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(std::move(memStream)));
    juce::int64 totalSamples = reader->lengthInSamples;
    vinylBuffer.setSize(reader->numChannels, static_cast<int>(totalSamples));    
    reader->read(vinylBuffer.getArrayOfWritePointers(), vinylBuffer.getNumChannels(), 0, static_cast<int>(totalSamples));
    DBG("First value in vinylBuffer: " << vinylBuffer.getReadPointer(0)[0]);
    DBG("num vinyl hannels: " << vinylBuffer.getNumChannels());
    
    
    
    
    
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

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    
    

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        // size of new blocks resampling
        float subblockSize = apvts.getRawParameterValue(blockSizeParamID.getParamID())->load(); // THIS WILL BE A PARAMETER
        int numSamples = buffer.getNumSamples();
        
        // check parameter here
        if (apvts.getRawParameterValue(lpPreParamID.getParamID())->load() && subblockSize > 1) {
            
            // Anti-Aliasing:
            // create LP cutoff at the nyquist frequency: current sample rate / subblock size is the resulting sample rate
            // resulting sample rate / 2 is the highest frequency that won't result in aliasing according to the Nyquist theorem
            
            float cutoffFrequency = (getSampleRate() / subblockSize / 2) ;
            *lowPassFilterPre.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(spec.sampleRate, cutoffFrequency);
            
            // Process the audio buffer with the updated filter
            juce::dsp::AudioBlock<float> audioBlock(buffer);
            auto channelBlock = audioBlock.getSingleChannelBlock(channel);
            juce::dsp::ProcessContextReplacing<float> context(channelBlock);
            lowPassFilterPre.process(context);
        }
        
        
        // resample at lower rate
        if (subblockSize > 1)
        {
            bitcrushSamples(buffer, subblockSize, channel);
            
        }
        
        
        if (apvts.getRawParameterValue(lpPostParamID.getParamID())->load() && subblockSize > 1) {
            // Post LP filter - after bitcrushing
        
            float cutoffFrequency = (getSampleRate() / subblockSize / 2) ;
            *lowPassFilterPost.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(spec.sampleRate, cutoffFrequency);
            
            // Process the audio buffer with the updated filter
            juce::dsp::AudioBlock<float> audioBlock(buffer);
            auto channelBlock = audioBlock.getSingleChannelBlock(channel);
            juce::dsp::ProcessContextReplacing<float> context(channelBlock);
            lowPassFilterPost.process(context);
        }
        
        
        if (subblockSize > 1)
        {
            //fade in/out
            
            // in
            int fadeLength = 32;
            auto* channelData = buffer.getWritePointer (channel);
            for (int i = 0; i < fadeLength; ++i) {
                float fadeFactor = static_cast<float>(i) / fadeLength;
                channelData[i] *= fadeFactor;
            }
            
            // out
            for (int i = numSamples - fadeLength; i < numSamples; ++i) {
                float fadeFactor = static_cast<float>(numSamples - i) / fadeLength;
                channelData[i] *= fadeFactor;
            }
        }
        
        
        
        
        
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
    layout.add(std::make_unique<juce::AudioParameterFloat>(blockSizeParamID, "Bitcrush Amount", 1.0, 20.0, 1.0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(noiseLevelParamID, "Noise Level", 0.0, 1.0, 0.5));
    layout.add(std::make_unique<juce::AudioParameterInt>(noiseTypeParamID, "Noise Type", 0, 1, 0));
    
    return layout;
}
