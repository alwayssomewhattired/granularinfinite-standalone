
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
    void processSamplerPath(juce::AudioBuffer<float>& buffer, const int& outCh, const int& numSamples);
    void processGranularPath(juce::AudioBuffer<float>& buffer, const int& outCh, const int& numSamples);
    void spawnGrain();

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
    void loadFile(const juce::File& file, const juce::String& noteName, std::optional<juce::String> fileName = std::nullopt);
    void startPlayback(const juce::String& note);
    void stopPlayback(const juce::String& note);
    void addMidiEvent(const juce::MidiMessage& m);

    bool synthToggle = false;
    bool grainAll = false;




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
    juce::Synthesiser synth;

    std::map<juce::String, std::unique_ptr<Sample>> samples;
    juce::AudioFormatManager formatManager;
    juce::AudioBuffer<float> tempBuffer;

    double m_sampleRate = 48000.0;
    int m_blockSize = 576;

    // make all grain members controllable

    struct Grain
    {
        int startSample;
        int position = 0;
        int length;
        float pitchRatio = 1.0f;

        const float* envelope = nullptr;
    };
    std::vector<Grain> grains;
    int grainCounter = 0;
    int grainSpacing = 256;
    int minGrainLength = 128;
    int maxGrainLength = 512; 
    int maxCircularSize = 24000; // half second

    std::vector<float> hannWindow;
    juce::AudioBuffer<float> circularBuffer;
    int circularWritePos = 0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularinfiniteAudioProcessor)
};
