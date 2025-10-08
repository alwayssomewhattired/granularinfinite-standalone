
#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class GranularinfiniteAudioProcessor  : public juce::AudioProcessor,
                                        public juce::ChangeBroadcaster
{
public:
    //==============================================================================
    GranularinfiniteAudioProcessor();
    ~GranularinfiniteAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    juce::AudioProcessorValueTreeState apvts;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    //void createParametersRange(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params);
    
   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processSamplerPath(juce::AudioBuffer<float>& buffer, const int& outCh, const int& numSamples);
    void processGranularPath(juce::AudioBuffer<float>& buffer, const int& outCh, const int& numSamples);
    void spawnGrain(int64_t fileLength);

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

    float getMaxFileSize() const;
    void  updateMaxFileSize(float const& newMaxFileSize);

    juce::AudioBuffer<float>& getSampleBuffer() const;

    bool synthToggle = false;
    bool grainAll = false;




private:
    struct Sample
    {
        std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
        juce::AudioTransportSource transportSource;
        bool isPrepared = false;

        std::int64_t audioFileLength = 0;
        juce::AudioBuffer<float> fullBuffer;

        void setSourceFromReader(juce::AudioFormatReader* reader)
        {
            readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
            transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);

            audioFileLength = reader->lengthInSamples;

            fullBuffer.setSize((int)reader->numChannels, (int)audioFileLength);

            reader->read(&fullBuffer, 0, (int)audioFileLength, 0, true, true);
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

    // make this dynamically hold the largest sample size
    float m_maxFileSize = 0.0;

    // make all grain members below controllable

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

    // controls
    int grainSpacing = 1;
    int grainAmount = 1;
    int minGrainLength = 128;
    std::atomic<float>* minGrainLengthPtr = nullptr;
    int maxGrainLength = 512; 
    std::atomic<float>* maxGrainLengthPtr = nullptr;
    int maxCircularSize = 24000; // half second
    juce::NormalisableRange<float> m_dynamicRange;

    std::vector<float> hannWindow;

    // dunno if i need this circular anymore
    juce::AudioBuffer<float> circularBuffer;
    int circularWritePos = 0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularinfiniteAudioProcessor)
};
