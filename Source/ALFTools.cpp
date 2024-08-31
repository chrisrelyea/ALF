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
}

juce::AudioBuffer<float> loadWavFile(juce::MemoryInputStream& memoryStream) {
    
}
