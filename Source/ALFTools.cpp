/*
  ==============================================================================

    ALFTools.cpp
    Created: 23 Jun 2024 10:54:39pm
    Author:  Chris Relyea

  ==============================================================================
*/
#include "ALFTools.h"
#include "BinaryData.h"

void bitcrushSamples(juce::AudioBuffer<float>& buffer, float blockSize, int channel) {
    
    auto* channelData = buffer.getWritePointer (channel);
    float currentVal = 0.0f;
    int numSamples = buffer.getNumSamples();
    int intBlockSize = static_cast<int>(blockSize);
    if (blockSize == intBlockSize)
    {
        for (int i = 0; i < numSamples; i++) {
            if (i % intBlockSize == 0) {
                currentVal = channelData[i];
            } else {
                channelData[i] = currentVal;
            }
        }
    }
    else
    {
        // set the two possible block sizes (rounding up and rounding down)
        int highBlockSize = intBlockSize + 1;
        int lowBlockSize = intBlockSize;
        float blockSizeDecimal = blockSize - intBlockSize; // decimal value of the float block size
        // loop through all samples in the buffer
        for (int i = 0; i < numSamples;) {
            float randomFloat = juce::Random::getSystemRandom().nextFloat();
            
            /* random float is compared to the decimal value to get the next block size
            * example: a size of 3.1f would compare a random float from 0.0 to 1.0 to the 0.1 decimal.
            * If the random float is higher than 0.1, the the lower block size is chosen. This makes sense because
            * 3.1 is much closer to 3.0 than 4.0, and thus the block size of 3 will be chosen most of the time. */
            int randomizedBlockSize = (randomFloat >= blockSizeDecimal) ? lowBlockSize : highBlockSize;
            
            int blockEnd = std::min(i + randomizedBlockSize, numSamples); // make sure we don't go out of bounds
            // fill in x values to be the value at the start of the block, where x is the randomly chosen block size
            for (int j = i; j < blockEnd; ++j) {
                channelData[j] = channelData[i];
            }
            // advance the iterator to the end of this block to get ready for the next
            i = blockEnd;
        }
    }
    
    
    // Fade in and out of each buffer to avoid clicks
    int fadeLength = 64;
    for (int i = 0; i < fadeLength; ++i) {
        float fadeFactor = static_cast<float>(i) / fadeLength;
        channelData[i] *= fadeFactor;
    }
    for (int i = numSamples - fadeLength; i < numSamples; ++i) {
        float fadeFactor = static_cast<float>(numSamples - i) / fadeLength;
        channelData[i] *= fadeFactor;
    }
}

void applyFilter(int channel, float subblockSize, double sampleRate, 
                 juce::dsp::IIR::Filter<float>& lowPassFilter, juce::dsp::ProcessSpec spec, juce::AudioBuffer<float>& buffer) {
    
    float cutoffFrequency = (sampleRate / subblockSize / 2) ;    // Nyquist frequency
    *lowPassFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(spec.sampleRate, cutoffFrequency);
    juce::dsp::AudioBlock<float> audioBlock(buffer);
    auto channelBlock = audioBlock.getSingleChannelBlock(channel);
    juce::dsp::ProcessContextReplacing<float> context(channelBlock);
    lowPassFilter.process(context);
}

void changeBitDepth(juce::AudioBuffer<float>& buffer, int channel, int paramVal) {
    // options are 24, 16, 12, 8, 4 bit
    int bitDepthVal;
    
    switch (paramVal) {
        case 0:
            bitDepthVal = 24;
            break;
        case 1:
            bitDepthVal = 16;
            break;
        case 2:
            bitDepthVal = 12;
            break;
        case 3:
            bitDepthVal = 8;
            break;
        case 4:
            bitDepthVal = 4;
            break;
    }
    
    auto* channelData = buffer.getWritePointer (channel);
    int numSamples = buffer.getNumSamples();
    int possibleVals = pow(2, bitDepthVal);
    DBG("new bit depth is " << bitDepthVal);
    DBG("poss vals is " << possibleVals);
    
    
    /* For each value in the buffer, convert it to the value that it would take in a system using the
     new bit depth (fewer distinct values between -1.0 and 1.0)
     1. Multiply original value by the number of possible vals of the new bit depth
     2. Round to nearest integer
     3. Divide by number of possible vals to re-normalize to -1.0 to 1.0 range */
    
    for (int i = 0; i < numSamples; i++) {
        channelData[i] = std::round(channelData[i] * possibleVals) / possibleVals;
    }
    
}


