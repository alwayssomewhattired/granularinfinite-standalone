
#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class GranularinfiniteAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    GranularinfiniteAudioProcessor();
    ~GranularinfiniteAudioProcessor() override;

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

    //==============================================================================
    void loadFile(const juce::File& file, const juce::String& noteName);
    void startPlayback(const juce::String& note);
    void stopPlayback(const juce::String& note);
    //void injectNoteOn(juce::MidiBuffer& midiMessages, const int& midiNote);
    //void injectNoteOff(juce::MidiBuffer& midiMessages, const int& midiNote);
    void addMidiEvent(const juce::MidiMessage& m);

    //bool isPrepared = false;
    bool synthToggle = false;



private:
    struct Sample
    {
        std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
        juce::AudioTransportSource transportSource;
        bool isPrepared = false;

        void setSourceFromReader(juce::AudioFormatReader* reader)
        {
            readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
            transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
        }
    };
    juce::MidiBuffer midiFifo;
    std::mutex midiMutex;
    std::map<juce::String, std::unique_ptr<Sample>> samples;
    juce::Synthesiser synth;
    juce::AudioFormatManager formatManager;
    juce::AudioBuffer<float> tempBuffer;

    double m_sampleRate = 48000.0;
    int m_blockSize = 576;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularinfiniteAudioProcessor)
};
