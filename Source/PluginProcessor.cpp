

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "OpenConsole.h"
#include "components/KeyButton.h"
#include "KeyToNote.h"
//#include "SpotifyAuthenticator.h"
//#include "SamplerInfinite.h"
//#include "PythonSoulseek.h"
#include <juce_dsp/juce_dsp.h>
#include <juce_events/juce_events.h>
#include <algorithm>
#include <iterator>




//==============================================================================
GranularinfiniteAudioProcessor::GranularinfiniteAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    apvts(*this, nullptr, "PARAMETERS", createParameters())
    //auth(),
    //spotifyAuthToken()
#endif
{
    minGrainLengthPtr = apvts.getRawParameterValue("grainMinLength");
    jassert(minGrainLengthPtr != nullptr);
    maxGrainLengthPtr = apvts.getRawParameterValue("grainMaxLength");
    jassert(maxGrainLengthPtr != nullptr);

    formatManager.registerBasicFormats();
    
    for (int i = 0; i < 16; ++i)
        synth.addVoice(new juce::SamplerVoice());

    outputBuffer.setSize(1, 1024);

    apvts.addParameterListener("compressorThreshold", this);
    apvts.addParameterListener("compressorRatio", this);

}

GranularinfiniteAudioProcessor::~GranularinfiniteAudioProcessor()
{
    //spotifyFetcher->stopFetching();

    // possibly make this a thing? dunno bc i already have 'stopFetching' method. prolly don't need this
    //spotifyFetcher->reset();

    apvts.removeParameterListener("compressorThreshold", this);
    apvts.removeParameterListener("compressorRatio", this);
}

//==============================================================================
const juce::String GranularinfiniteAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GranularinfiniteAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GranularinfiniteAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GranularinfiniteAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GranularinfiniteAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GranularinfiniteAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GranularinfiniteAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GranularinfiniteAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GranularinfiniteAudioProcessor::getProgramName (int index)
{
    return {};
}

void GranularinfiniteAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================

void GranularinfiniteAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // prepares samples for juce 'synth' mode
    /*if (!m_grainAll) {
        for (auto& pair : samples)
        {
            if (std::find(currentFiles.begin(), currentFiles.end(), pair.first.second) != currentFiles.end())
                pair.second->transportSource.prepareToPlay(samplesPerBlock, sampleRate);
            std::cout << "totalLength: " << pair.second->transportSource.getTotalLength() << "\n";
            break;
        }

        synth.setCurrentPlaybackSampleRate(sampleRate);
    }*/

    // do i really need to do this?
    //m_sampleRate = sampleRate;
    //m_blockSize = samplesPerBlock;
    
    //synth.setCurrentPlaybackSampleRate(sampleRate);

    // i beleive temp buffer and circular buffer can GTFO
    tempBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock, false, false, true);
    tempBuffer.clear();

    circularBuffer.setSize(getTotalNumOutputChannels(), maxCircularSize, false, false, true);
    circularBuffer.clear();

    // make control for hann window
    m_hannWindowExperiment.resize(m_maxGrainLength);
    juce::dsp::WindowingFunction<float>::fillWindowingTables(
        m_hannWindowExperiment.data(),
        m_hannWindowExperiment.size(),
        juce::dsp::WindowingFunction<float>::hann
    );


    // frequency upward-compressor

    juce::dsp::ProcessSpec spec{ sampleRate, (juce::uint32)samplesPerBlock, 1 };

    bandpassFilter.prepare(spec);
    m_upwardCompressor.prepare(spec);

    // compressor settings
    m_upwardCompressor.setThreshold(-40.0f);
    m_upwardCompressor.setRatio(10.0f);
    m_upwardCompressor.setAttack(10.0f);
    m_upwardCompressor.setRelease(50.0f);

    //set center frequency at startup

    updateFilter(sampleRate);



    //
    // //
    //// trigger this via button click in the future
    //////spotify token grabber
    //SpotifyAuthenticator auth;
    //auth.init("8df0570e51ae419baf4a7e2845a43cb4", "5aae9f994086437696de02533fd96ebd", "http://127.0.0.1:8888/callback");
    //auth.startAuthentication();
    //spotifyAuthToken = auth.waitAndGetToken();
    //std::cout << spotifyAuthToken << "\n";

    //spotifyFetcher = std::make_unique<SamplerInfinite>(spotifyAuthToken);

    //spotifyFetcher->onSongsFetched = [this](const juce::StringArray& songs)
    //    {
    //        for (auto& s : songs)
    //            std::cout << "song: " << s << "\n";
    //    };

    ////// trigger this via button click in the future
    //spotifyFetcher->startFetching();
    //std::cout << "all done\n";
}

void GranularinfiniteAudioProcessor::releaseResources()
{
    // releases resources of juce 'synth' mode
   /* for (auto& pair : samples)
    {
        if (std::find(currentFiles.begin(), currentFiles.end(), pair.first.first) != currentFiles.end())
            pair.second->transportSource.releaseResources();
        break;
    }*/
    }

#ifndef JucePlugin_PreferredChannelConfigurations
bool GranularinfiniteAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


juce::AudioProcessorValueTreeState::ParameterLayout GranularinfiniteAudioProcessor::createParameters()
{

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "serialSchedule",
        "SerialSchedule",
        false
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "grainDensity",
        "GrainDensity",
        juce::NormalisableRange<float> (1.0f, 100.0f, 1.0f),
        1.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "grainAmount",
        "GrainAmount",
        juce::NormalisableRange<float> (1.0f, 256.0f, 1.0f),
        1.0f
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "chunkCrossfade",
        "ChunkCrossfade",
        0,
        576,
        0
    ));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "grainMaxLength",
        "GrainMaxLength",
        128,
        48000,
        36000
    ));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "grainMinLength",
        "GrainMinLength",
        128,
        48000,
        512
    ));

    auto dynamicRange = juce::NormalisableRange<float>(
        0.0f,
        100.0f
    );

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "grainMinArea",
        "GrainMinArea",
        dynamicRange,
        0
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "grainMaxArea",
        "GrainMaxArea",
        dynamicRange,
        0
    ));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "hanningToggle",
        "HanningToggle",
        false
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "globalGain",
        "GlobalGain",
        0.00f,
        1.00f,
        0.10f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compressorThreshold",
        "CompressorThreshold",
        juce::NormalisableRange<float>(0.001f, 1.0f), 
        0.1f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float v, int)
            {
                return juce::String(v, 3);  // <-- 3 decimal places
            })
        .withValueFromStringFunction([](const juce::String& s)
            {
                return s.getFloatValue();
            })
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compressorRatio",
        "CompressorRatio",
        1.0f,
        20.0f,
        1.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compressorAttackCoeff",
        "CompressorAttackCoeff",
        0.1f,
        50.0f,
        10.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compressorReleaseCoeff",
        "CompressorReleaseCoeff",
        20.0f,
        2000.0f,
        200.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compressorGain",
        "CompressorGain",
        0.01f,
        1.0f,
        1.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compressorMix",
        "CompressorMix",
        0.0f,
        1.0f,
        0.0f
    ));


    // loop over every note name for this
    for (const auto& note : allNotes()) {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            std::string("frequencyUpwardCompressorProminence") + std::string(note),
            std::string("FrequencyUpwardCompressorProminence") + std::string(note),
            0.00f,
            1.00f,
            0.00f
        ));
    }

    return { params.begin(), params.end() };
}

void GranularinfiniteAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue) {
    if (parameterID == "compressorThreshold") {
        if (onThresholdChanged) {
            juce::MessageManager::callAsync([this, newValue] {
                onThresholdChanged(newValue);
                });
        }
    }
    else if (parameterID == "compressorRatio") {
        if (onRatioChanged) {
            juce::MessageManager::callAsync([this, newValue] {
                onRatioChanged(newValue);
                });
        }
    }
}

void GranularinfiniteAudioProcessor::updateMaxFileSize(float const& newMaxFileSize)
{
    std::cout << newMaxFileSize << "\n";
    if (m_maxFileSize < newMaxFileSize) {
        m_maxFileSize = newMaxFileSize;
    }
}

// this either removes or adds a note to the currentNotes vector
void GranularinfiniteAudioProcessor::updateCurrentSamples(const juce::String noteName, const bool remove) {
    if (!remove)
        currentNotes.push_back(noteName);
    else {
        currentNotes.erase(std::remove(currentNotes.begin(), currentNotes.end(), noteName), currentNotes.end());
    }
}

void GranularinfiniteAudioProcessor::addMidiEvent(const juce::MidiMessage& m)
{
    const std::lock_guard<std::mutex> lock(midiMutex);
    midiFifo.addEvent(m, 0);
}


void GranularinfiniteAudioProcessor::spawnGrain(int64_t fileLength, const bool& isFuture)
{

    const bool chunkCrossfade =
        apvts.getRawParameterValue("chunkCrossfade")->load();

    const size_t maxGrains =
        chunkCrossfade ? grainAmount + 1 : grainAmount;

    if (grains.size() >= maxGrains && m_isSerialSchedule)
    {
        return;
    }

    Grain g;
    g.length = juce::Random::getSystemRandom().nextInt(m_maxGrainLength - minGrainLength + 1) + minGrainLength;
    
    // - this isn't great. we should be getting our normalized values from apvts... not normalizing them here
    m_maxGrainLength = apvts.getRawParameterValue("grainMaxLength")->load();
    const int64_t grainAreaMax =
        static_cast<int64_t>(
            (apvts.getRawParameterValue("grainMaxArea")->load() / 100)
            * static_cast<double>(fileLength)
            );
    const int64_t grainAreaMin =
        static_cast<int64_t>(
            (apvts.getRawParameterValue("grainMinArea")->load() / 100)
            * static_cast<double>(fileLength)
            );

    const int range = std::max(1, (int)(grainAreaMax - grainAreaMin - g.length));

    g.startSample = grainAreaMin + juce::Random::getSystemRandom().nextInt(range);

    g.position = 0;

    // | if future grain, move future grain to current grain 
    if (isFuture) {
        m_futureGrains.push_back(std::move(g));
    }
    else {
    // - insert pitch stuff here...
    grains.push_back(std::move(g));

    }
}


void GranularinfiniteAudioProcessor::processSamplerPath(juce::AudioBuffer<float>& buffer, const int& outCh, const int& numSamples)
{

    buffer.clear();

    // ** current note name
    for (const juce::String& noteName : currentNotes) {
        for (auto& pair : samples)
        {
            if (pair.first.first != noteName) continue;

      
            auto& sample = pair.second;
            if (!sample->isChosen) {
                continue;
            }

            auto& sourceBuffer = sample->fullBuffer;
            int sourceLength = sourceBuffer.getNumSamples();
            const float globalGain = apvts.getRawParameterValue("globalGain")->load();

            for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx)
            {
                if (sample->readIndex >= sourceLength) {
                    sample->readIndex = 0;
                }

                float limiterSample = limiter(sourceBuffer.getSample(0, sample->readIndex), 0.8f) * globalGain;

                for (int ch = 0; ch < outCh; ++ch)
                {
                    buffer.addSample(ch, sampleIdx, limiterSample);
                }

                sample->readIndex++;
            }
        }
    }
}

void GranularinfiniteAudioProcessor::processGranularPath(juce::AudioBuffer<float>& buffer, const int& outCh, const int& chunkSize)
{
    buffer.clear();
    minGrainLength = *minGrainLengthPtr;
    m_maxGrainLength = *maxGrainLengthPtr;
    initHann(m_maxGrainLength);

    // ** current note name
    for (const juce::String& noteName : currentNotes) {
        for (auto& pair : samples)
        {
            if (pair.first.first != noteName) continue;

            auto& sample = pair.second;
            if (!sample->isChosen) {
                continue;
            }

            auto& m_fullBuffer = sample->fullBuffer;

            // | begin sample processing
            for (int sampleIdx = 0; sampleIdx < chunkSize; ++sampleIdx)
            {
                float out = 0.0f;

                // | overlapping schedule block
                if (!m_isSerialSchedule) {
                    grainCounter++;
                    if (grainCounter >= m_grainDensity) {
                        grainCounter = 0;
                        spawnGrain(sample->audioFileLength, false);
                    }
                }
                // | end overlapping schedule block

                for (size_t i = 0; i < grains.size(); )
                {
                    Grain& g = grains[i];
                    if (g.position < g.length)
                    {
                        int fullBufferIdx = (g.startSample + (int)(g.position * g.pitchRatio));
                        if (fullBufferIdx >= m_fullBuffer.getNumSamples() || fullBufferIdx < 0)
                        {
                            grains.erase(grains.begin() + i);
                            continue;
                        }

                        float currentSample = m_fullBuffer.getSample(0, fullBufferIdx);

                        // | window-overlap block
                        if (const bool hanningToggle = apvts.getRawParameterValue("hanningToggle")->load()) {
                            if (fullBufferIdx < 0 || fullBufferIdx >= m_fullBuffer.getNumSamples()) {
                                grains.erase(grains.begin() + i);
                                continue;
                            }

                            float phase = g.position / float(g.length);
                            int windowIdx = int(phase * (m_hannWindowOverlap.size() - 1));
                            windowIdx = juce::jlimit(0, (int)m_hannWindowOverlap.size() - 1, windowIdx);
                            currentSample *= m_hannWindowOverlap[windowIdx];
                        }
                        // | end window overlap block


                        // | chunk crossfade block
                        int chunkCrossfadeAmount = apvts.getRawParameterValue("chunkCrossfade")->load();
                        int fadeStartPos = g.position - (chunkSize - chunkCrossfadeAmount);
                        if (chunkCrossfadeAmount > 0 && fadeStartPos >= 0 && i < grains.size() && m_futureGrains.size() > i ) {
                            Grain& futureGrain = m_futureGrains[i];

                            int futureReadPos = futureGrain.startSample + int(fadeStartPos * futureGrain.pitchRatio);

                            float futureSample = m_fullBuffer.getSample(0, futureReadPos);

                            if (futureReadPos < 0 || futureReadPos >= m_fullBuffer.getNumSamples()) {
                                std::cout << "oh no thats not good\n";
                                futureSample = currentSample;  // safe fallback
                            }
                            currentSample = chunkCrossFade(g.position, chunkSize, currentSample, futureSample, chunkCrossfadeAmount);
                        }
                        // | end chunk crossfade block

                        const float globalGain = apvts.getRawParameterValue("globalGain")->load();

                        updateCompressor();
                        float limiterSample = limiter(currentSample, 0.8f);

                        float upwardCompressed = upwardCompressor(limiterSample, noteName.toStdString());

                        int pos = writePos.load(std::memory_order_relaxed);

                        outputBuffer.setSample(0, pos, limiterSample);

                        pos = (pos + 1) % 1024;
                        writePos.store(pos, std::memory_order_release);

                        out += (m_compressor.process(upwardCompressed)) * globalGain;

                        ++g.position;
                        ++i;

                    }
                    // | erases current grain from grain vector when finitshed with grain
                    else {

                        grains.erase(grains.begin() + i);
                    }

                }
                for (int ch = 0; ch < outCh; ++ch)
                {
                    buffer.addSample(ch, sampleIdx, out);
                }
            }
            // | end sample processing

            // | serial scheduler block
            // - expose control for this
            if (m_isSerialSchedule) {
                grainCounter += chunkSize;
                if (grainCounter >= m_grainDensity)
                {
                    grainCounter = 0;

                    if (apvts.getRawParameterValue("chunkCrossfade")->load()) {
                        if (m_futureGrains.size() > 0) {
                            grains.push_back(std::move(m_futureGrains[0]));
                            m_futureGrains.erase(m_futureGrains.begin());
                            grains.erase(grains.begin());
                            spawnGrain(sample->audioFileLength, true);
                            continue;
                        }
                        else {
                            spawnGrain(sample->audioFileLength, true);
                            continue;
                        }
                    }

                    spawnGrain(sample->audioFileLength, false);
                }
            }
            // | end serial scheduler block
        }
    }
}

// | Hann window for overlap
void GranularinfiniteAudioProcessor::initHann(int maxGrainSize) {
    m_hannWindowOverlap.resize(maxGrainSize);
    for (int i = 0; i < maxGrainSize; ++i) {
        m_hannWindowOverlap[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (maxGrainSize - 1)));
    }
}

float GranularinfiniteAudioProcessor::chunkCrossFade(
    int grainPos,
    int grainLength,
    float currentSample,
    float futureSample,
    int chunkCrossfadeAmount)
{

    if (chunkCrossfadeAmount <= 0 || chunkCrossfadeAmount > grainLength) {
        return currentSample;  // nothing to crossfade
    }

    int fadeStart = grainLength - chunkCrossfadeAmount;

    if (grainPos < fadeStart) {
        return currentSample;
    }

    float t = float(grainPos - fadeStart) / float(chunkCrossfadeAmount);

    t = juce::jlimit(0.0f, 1.0f, t);

    float fadeIn  = std::sin(0.5f * juce::MathConstants<float>::pi * t);
    float fadeOut = std::cos(0.5f * juce::MathConstants<float>::pi * t);

    return fadeOut * currentSample + fadeIn * futureSample;
}


float GranularinfiniteAudioProcessor::upwardCompressor(float x, const std::string& noteName)
{
    // get presence value
    float presence = apvts.getRawParameterValue("frequencyUpwardCompressorProminence" + noteName)->load();
    if (presence == 0.0f) return x;

    float makeupGain = juce::Decibels::decibelsToGain(presence * 12.0f);
    float wetDry = presence;

    // 1. Extract the frequency band
    float band = bandpassFilter.processSample(x);

    // 2. Run through downward compressor
    float before = band;

    // first channel only... forsseable change in future
    float after = m_upwardCompressor.processSample(0, band);

    // 3. Invert gain reduction (upward compression)
    float gainReduction = before - after;

    float upwardBoost = gainReduction * presence; // presenceAmount = presence

    // 4. Combine band + inverted GR
    float processed = after + upwardBoost;

    // Apply makeup gain only to the processed band
    processed *= makeupGain;

    // 5. Wet/dry mix into original signal
    return x + processed * wetDry;
}



void GranularinfiniteAudioProcessor::updateFilter(const double& sampleRate) {
    float Q = 10.0f;

    auto coeffs = juce::dsp::IIR::Coefficients<float>::makeBandPass(
        sampleRate,
        centerFreq,
        Q
    );

    bandpassFilter.coefficients = coeffs;
}

// simple limiter
float GranularinfiniteAudioProcessor::limiter(float x, float threshold) {
    if (x > threshold) return threshold + (x - threshold) * 0.2f;
    if (x < -threshold) return -threshold + (x + threshold) * 0.2f;
    return x;
}

void GranularinfiniteAudioProcessor::updateCompressor() {
    m_compressor.threshold = *apvts.getRawParameterValue("compressorThreshold");
    m_compressor.ratio = *apvts.getRawParameterValue("compressorRatio");
    m_compressor.attackCoeff = std::exp(-1.0f / (*apvts.getRawParameterValue("compressorAttackCoeff") * 0.001f * m_sampleRate));
    m_compressor.releaseCoeff = std::exp(-1.0f / (*apvts.getRawParameterValue("compressorReleaseCoeff") * 0.001f * m_sampleRate));
    m_compressor.gain = *apvts.getRawParameterValue("compressorGain");
    m_compressor.mix = *apvts.getRawParameterValue("compressorMix");
}

void GranularinfiniteAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const int outCh = buffer.getNumChannels();

    const int chunkSize = buffer.getNumSamples();

    // clear anything in extra output channels (host could provide input channels)
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, chunkSize);

    // make sure tempBuffer matches current block in case host block size changed
    if (tempBuffer.getNumChannels() != outCh || tempBuffer.getNumSamples() != chunkSize)
    {
        // setSize is safe here because we only call it when shape changed
        tempBuffer.setSize(outCh, chunkSize, false, false, true);
    }

    // PARAMETER STUFF
    m_isSerialSchedule = (int)apvts.getRawParameterValue("serialSchedule")->load();

    m_grainDensity = static_cast<int>(apvts.getRawParameterValue("grainDensity")->load());
    m_grainDensity = int(std::round(m_sampleRate / m_grainDensity));
    m_grainDensity = juce::jlimit(1, (2 * m_grainDensity), m_grainDensity);

    grainAmount = static_cast<int>(apvts.getRawParameterValue("grainAmount")->load());
    minGrainLength = apvts.getRawParameterValue("grainMinLength")->load();
    m_maxGrainLength = apvts.getRawParameterValue("grainMaxLength")->load();

    // SYNTH PATH
    //if (synthToggle)
    //{
    //    {
    //        const std::lock_guard<std::mutex> lock(midiMutex);
    //        midiMessages.addEvents(midiFifo, 0, numSamples, 0);
    //        midiFifo.clear();
    //    }

    //    // render synth into cleared buffer
    //    buffer.clear();
    //    synth.renderNextBlock(buffer, midiMessages, 0, numSamples);
    //    return;
    //}


    if (m_grainAll)
    {
        if (currentFiles.size() > 0) {

            processGranularPath(buffer, outCh, chunkSize);
        }
        return;
    }
    else {
        if (m_keyPressed)
            processSamplerPath(buffer, outCh, chunkSize);
        return;
    }
}

//////////////////////////////////  GETTERS /////////////////////////////////////////////////////////

float GranularinfiniteAudioProcessor::getMaxFileSize() const
{
    return m_maxFileSize;

}

// takes notename and returns the  audio buffer
const juce::AudioBuffer<float>& GranularinfiniteAudioProcessor::getSampleBuffer(const juce::String& noteName, const juce::String& fileName) const
{
    auto it = samples.find({ noteName, fileName });
    if (it != samples.end())
    {
        return it->second->fullBuffer;
    }
    else {
        std::cout << "getSampleBuffer failed \n";
    }
}

// loads audio file on file drop
//      -make this function update the m_maxFileSize with the largest file size vvv
std::shared_ptr<Sample>& GranularinfiniteAudioProcessor::loadFile(const juce::File& file, const juce::String& noteName, 
    std::optional<juce::String> fileName)
{
    std::string realStr = noteName.toStdString();
    if (auto* reader = formatManager.createReaderFor(file))
    {
        synth.clearSounds();
        int rootNote = CreateNoteToMidi.at(noteName);
        juce::BigInteger allNotes;
        allNotes.setRange(0, 128, true);
        const int midi_note = CreateNoteToMidi[noteName];
        auto sound = new juce::SamplerSound(noteName.toStdString(), *reader, allNotes,
            rootNote, 0.0, 0.0, 10.0);
        synth.addSound(sound);

        auto sample = std::make_shared<Sample>();
        sample->setSourceFromReader(reader);
        updateMaxFileSize(sample->audioFileLength);
        sample->fileName = file.getFileNameWithoutExtension();
        sample->noteName = noteName;
        
        samples[{ noteName, file.getFileNameWithoutExtension() }] = sample;
        fileNameToSample[file.getFileNameWithoutExtension()] = sample;
        prepareToPlay(m_sampleRate, m_blockSize);
        return sample;
    }
}

void GranularinfiniteAudioProcessor::startPlayback(const juce::String& note, const juce::String& fileName)
{
    m_keyPressed = true;
    // todoS
    // make this function take a velocity control
    // 
    //if (synthToggle)
    //{
    //    const int midi_note = CreateNoteToMidi[note];
    //    synth.noteOn(1, midi_note, 127.0f);
    //    return;
    //}

    auto it = samples.find({ note, fileName });

    if (it != samples.end())
    {
        currentFiles.push_back(fileName);
        auto& sample = it->second;
        //if (!m_grainAll) {
        //    if (!sample->isPrepared)
        //    {
        //        sample->transportSource.prepareToPlay(48000.0, 576);
        //        sample->isPrepared = true;
        //    }
        //    if (!sample->transportSource.isPlaying())
        //    {
        //        prepareToPlay(m_sampleRate, m_blockSize);
        //        sample->transportSource.setPosition(0.0);
        //        sample->transportSource.start();
        //    }
        //}
        //else {
            prepareToPlay(m_sampleRate, m_blockSize);
        //}


    }
}

void GranularinfiniteAudioProcessor::stopPlayback(const juce::String& note, const juce::String& fileName)
{
    const int midi_note = CreateNoteToMidi[note];

  /*  if (synthToggle)
    {
        synth.noteOff(1, midi_note, 127, false);
    }*/

    auto it = samples.find({ note, fileName });
    if (it != samples.end())
    {
        m_keyPressed = false;
        //it->second->readIndex = 0;
        currentFiles.push_back(it->first.second);

   /*     if (!m_grainAll) {
            auto& sample = it->second;

            std::thread([samplePtr = sample.get()] {
                if (samplePtr)
                    samplePtr->transportSource.stop();
                }).detach();
        }*/
    }

}

//==============================================================================
bool GranularinfiniteAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GranularinfiniteAudioProcessor::createEditor()
{
    return new GranularinfiniteAudioProcessorEditor (*this);
}

//==============================================================================
void GranularinfiniteAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void GranularinfiniteAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GranularinfiniteAudioProcessor();
}

