
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
    void startPlayback(const juce::String& note);
    void stopPlayback(const juce::String& note);
    void addMidiEvent(const juce::MidiMessage& m);

    float getMaxFileSize() const;
    void  updateMaxFileSize(float const& newMaxFileSize);

    void updateCurrentSamples(const juce::String fileName, const bool remove);

    juce::AudioBuffer<float>& getSampleBuffer(const juce::String& fileName) const;


    // frequency compressor
    float upwardCompressor(float x, const std::string& notename);
    void updateFilter(const double& sampleRate);

    //juce::dsp::Compressor<float> m_compressor;
    juce::dsp::IIR::Filter<float> bandpassFilter;
    juce::dsp::Compressor<float> m_upwardCompressor;

    float centerFreq = 220.00f;
    float presenceAmount = 0;



    float limiter(float x, float threshold);

    bool synthToggle = false;

    // granular toggle flag
    bool grainAll = false;

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

    Sample* loadFile(const juce::File& file, const juce::String& noteName, std::optional<juce::String> fileName = std::nullopt);

private:
    //SpotifyAuthenticator auth;
    //juce::String spotifyAuthToken;

    //// samplerinfinite
    //std::unique_ptr<SamplerInfinite> spotifyFetcher;

    
    // flag for if key pressed
    bool m_keyPressed = false;

    juce::MidiBuffer midiFifo;
    std::mutex midiMutex;
    juce::Synthesiser synth;

    // file-name to Sample object
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
    int minGrainLength = 512;
    std::atomic<float>* minGrainLengthPtr = nullptr;
    int maxGrainLength = 3600; 
    std::atomic<float>* maxGrainLengthPtr = nullptr;
    int maxCircularSize = 24000; // half second
    juce::NormalisableRange<float> m_dynamicRange;

    std::vector<float> hannWindow;

    struct MySampleCompressor {
        float envelope = 0.0f; // leave at 0.0f;
        float threshold = 0.1f; // linear
        float ratio = 1.0f;
        float attackCoeff = 0.0f;
        float releaseCoeff = 0.0f;
        float gain = 1.0f;

        float process(float input) {
            float level = std::abs(input);

            // envelope follower
            if (level > envelope)
                envelope = attackCoeff * (envelope - level) + level;
            else
                envelope = releaseCoeff * (envelope - level) + level;

            if (envelope > threshold) {
                float over = envelope / threshold;
                float compressed = std::pow(over, -(ratio - 1.0f));
                gain = compressed;
            }

            float mix = 0.5f;

            float wet = input * gain;
            float dry = input;

            //return input * gain;
            return dry * (1.0f - mix) + wet * mix;
        }
    };

    MySampleCompressor m_compressor;

    void updateCompressor();

    // dunno if i need this circular anymore
    juce::AudioBuffer<float> circularBuffer;
    int circularWritePos = 0;

    // - stored noteNames of current samples
    std::vector<juce::String> currentNotes;

    // - stored fileNames of current samples
    std::vector<juce::String> currentFiles;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularinfiniteAudioProcessor)
};
