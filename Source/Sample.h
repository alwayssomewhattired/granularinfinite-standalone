
#pragma once
#include <memory>
#include <JuceHeader.h>

// non-copyable
struct Sample
{
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    std::int64_t audioFileLength = 0;
    juce::AudioBuffer<float> fullBuffer;

    juce::String fileName;
    juce::String noteName;

    int readIndex = 0;

    bool isChosen = false;

    // juce 'synth' mode
    bool isPrepared = false;

    void setSourceFromReader(juce::AudioFormatReader* reader)
    {
        readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);

        audioFileLength = reader->lengthInSamples;

        fullBuffer.setSize((int)reader->numChannels, (int)audioFileLength);

        reader->read(&fullBuffer, 0, (int)audioFileLength, 0, true, true);
    }
};