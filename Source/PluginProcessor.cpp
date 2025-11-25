

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "OpenConsole.h"
#include "KeyButton.h"
#include "KeyToNote.h"
//#include "SpotifyAuthenticator.h"
//#include "SamplerInfinite.h"
//#include "PythonSoulseek.h"
#include <juce_dsp/juce_dsp.h>
#include <juce_events/juce_events.h>
#include <algorithm>

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

}

GranularinfiniteAudioProcessor::~GranularinfiniteAudioProcessor()
{
    //spotifyFetcher->stopFetching();

    // possibly make this a thing? dunno bc i already have 'stopFetching' method. prolly don't need this
    //spotifyFetcher->reset();
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
    for (auto& pair : samples)
    {
        pair.second->transportSource.prepareToPlay(samplesPerBlock, sampleRate);
    }
    m_sampleRate = sampleRate;
    m_blockSize = samplesPerBlock;
    synth.setCurrentPlaybackSampleRate(sampleRate);

    // i beleive temp buffer and circular buffer can GTFO
    tempBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock, false, false, true);
    tempBuffer.clear();

    circularBuffer.setSize(getTotalNumOutputChannels(), maxCircularSize, false, false, true);
    circularBuffer.clear();

    hannWindow.resize(maxGrainLength);
    juce::dsp::WindowingFunction<float>::fillWindowingTables(
        hannWindow.data(),
        hannWindow.size(),
        juce::dsp::WindowingFunction<float>::hann
    );


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
    for (auto& pair : samples)
    {
        pair.second->transportSource.releaseResources();
    }
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

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "grainSpacing",
        "GrainSpacing",
        juce::NormalisableRange<float> (1.0f, 48000.0f, 1.0f),
        1.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "grainAmount",
        "GrainAmount",
        juce::NormalisableRange<float> (1.0f, 256.0f, 1.0f),
        1.0f
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
        600.0f
    );

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "grainPosition",
        "GrainPosition",
        dynamicRange,
        0
    ));

    return { params.begin(), params.end() };
}

void GranularinfiniteAudioProcessor::updateMaxFileSize(float const& newMaxFileSize)
{

    if (m_maxFileSize < newMaxFileSize) {
        m_maxFileSize = newMaxFileSize;
    }
    // I don't believe we need the rest of this down below...
    if (auto* param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("grainPosition")))
    {
        
        param->range = juce::NormalisableRange<float>(0.0f, m_maxFileSize);

        // this converts the native range(user) to normalized(0-1)
        auto clamped = juce::jlimit(0.0f, m_maxFileSize, param->get());
        auto normalized = param->convertTo0to1(clamped);
        param->setValueNotifyingHost(normalized);
        param->setValueNotifyingHost(normalized); // force refresh?
    }

    sendChangeMessage();
}

void GranularinfiniteAudioProcessor::updateCurrentSamples(const juce::String noteName, const bool remove) {
    // samples is duplicating when repressed. fix this. could be in another function
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


void GranularinfiniteAudioProcessor::spawnGrain(int64_t fileLength)
{

    if ((int)grains.size() >= grainAmount)
        return;


    Grain g;

    g.length = juce::Random::getSystemRandom().nextInt(maxGrainLength - minGrainLength + 1) + minGrainLength;
    // start sample plays a random sample
    maxGrainLength = apvts.getRawParameterValue("grainMaxLength")->load();
    const float grainArea = (apvts.getRawParameterValue("grainPosition")->load() / 600.0f) * m_maxFileSize;

    g.startSample = juce::Random::getSystemRandom().nextInt(std::abs(grainArea - g.length));

    g.position = 0;
    // insert pitch stuff here...
    grains.push_back(std::move(g));
}


void GranularinfiniteAudioProcessor::processSamplerPath(juce::AudioBuffer<float>& buffer, const int& outCh, const int& numSamples)
{

    for (auto& pair : samples)
    {
        auto& sample = pair.second;

        tempBuffer.setSize(outCh, numSamples, false, false, true);
        tempBuffer.clear();

        juce::AudioSourceChannelInfo info(&tempBuffer, 0, numSamples);
        sample->transportSource.getNextAudioBlock(info);

        const int srcChans = tempBuffer.getNumChannels();
        const int chansToMix = std::min(outCh, srcChans);

        for (int ch = 0; ch < chansToMix; ++ch)
            buffer.addFrom(ch, 0, tempBuffer, ch, 0, numSamples);

        if (srcChans == 1 && outCh >= 2)
            buffer.addFrom(1, 0, tempBuffer, 0, 0, numSamples);
    }
}

void GranularinfiniteAudioProcessor::processGranularPath(juce::AudioBuffer<float>& buffer, const int& outCh, const int& numSamples)
{
    buffer.clear();
    minGrainLength = *minGrainLengthPtr;
    maxGrainLength = *maxGrainLengthPtr;
    // *TO-DO*
    // - iterate over the correct sample that's pressed
    for (const juce::String& noteName : currentNotes) {
        for (auto& pair : samples)
        {

            // *TO-DO*
            // - we never pass this condition
            //std::cout << "first: " << pair.first.toStdString() << "\n";
            //std::cout << "noteName: " << noteName.toStdString() << "\n";
            if (pair.first != noteName) continue;

            auto& sample = pair.second;
            auto& m_fullBuffer = sample->fullBuffer;

            for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx)
            {
                float out = 0.0f;

                for (auto it = grains.begin(); it != grains.end(); )
                {
                    Grain& g = *it;
                    if (g.position < g.length)
                    {
                        int readIndex = (g.startSample + (int)(g.position * g.pitchRatio));
                        if (readIndex >= m_fullBuffer.getNumSamples())
                        {
                            it = grains.erase(it);
                            continue;
                        }

                        int idx = (g.position * hannWindow.size()) / g.length;
                        float env = hannWindow[idx];
                        out += m_fullBuffer.getSample(0, readIndex) * env;

                        ++g.position;
                        ++it;
                    }
                    else {
                        it = grains.erase(it);
                    }
                }
                for (int ch = 0; ch < outCh; ++ch)
                {
                    buffer.addSample(ch, sampleIdx, out);
                }
            }

            grainCounter += numSamples;
            if (grainCounter >= grainSpacing)
            {
                grainCounter = 0;
                spawnGrain(sample->audioFileLength);
            }
        }
    }
}

void GranularinfiniteAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const int outCh = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // clear anything in extra output channels (host could provide input channels)
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    // make sure tempBuffer matches current block in case host block size changed
    if (tempBuffer.getNumChannels() != outCh || tempBuffer.getNumSamples() != numSamples)
    {
        // setSize is safe here because we only call it when shape changed
        tempBuffer.setSize(outCh, numSamples, false, false, true);
    }
    // PARAMETER STUFF
    grainSpacing = static_cast<int>(apvts.getRawParameterValue("grainSpacing")->load());
    grainAmount = static_cast<int>(apvts.getRawParameterValue("grainAmount")->load());
    minGrainLength = apvts.getRawParameterValue("grainMinLength")->load();
    maxGrainLength = apvts.getRawParameterValue("grainMaxLength")->load();

    // SYNTH PATH
    if (synthToggle)
    {
        {
            const std::lock_guard<std::mutex> lock(midiMutex);
            midiMessages.addEvents(midiFifo, 0, numSamples, 0);
            midiFifo.clear();
        }

        // render synth into cleared buffer
        buffer.clear();
        synth.renderNextBlock(buffer, midiMessages, 0, numSamples);
        return;
    }


    if (grainAll)
    {
        // pass the buttons pressed into granularpath
        if (m_keyPressed)
            processGranularPath(buffer, outCh, numSamples);
        return;
    }
    else {
        if (m_keyPressed)
            processSamplerPath(buffer, outCh, numSamples);
        return;
    }
}

//////////////////////////////////  GETTERS /////////////////////////////////////////////////////////

float GranularinfiniteAudioProcessor::getMaxFileSize() const
{
    return m_maxFileSize;

}
// takes notename and returns the  audio buffer
juce::AudioBuffer<float>& GranularinfiniteAudioProcessor::getSampleBuffer(const juce::String& noteName) const
{
    auto it = samples.find(noteName);
    if (it != samples.end())
    {
        return it->second->fullBuffer;
    }
    else {
        std::cout << "getSampleBuffer failed \n";
    }
}

// make this function update the m_maxFileSize with the largest file size vvv
GranularinfiniteAudioProcessor::Sample* GranularinfiniteAudioProcessor::loadFile(const juce::File& file, const juce::String& noteName, 
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
        auto sample = std::make_unique<Sample>();
        sample->setSourceFromReader(reader);
        updateMaxFileSize(sample->audioFileLength);

        auto* samplePtr = sample.get();
        samples[noteName] = std::move(sample);
        return samplePtr;
    }
}

void GranularinfiniteAudioProcessor::startPlayback(const juce::String& note)
{
    m_keyPressed = true;
    // todoS
    // make this function take a velocity control
    if (synthToggle)
    {
        const int midi_note = CreateNoteToMidi[note];
        synth.noteOn(1, midi_note, 127.0f);
        return;
    }

    auto it = samples.find(note);
    if (it != samples.end())
    {
        auto& sample = it->second;
        if (!sample->isPrepared)
        {
            sample->transportSource.prepareToPlay(48000.0, 576);
            sample->isPrepared = true;
        }
        if (!sample->transportSource.isPlaying())
        {
            
            prepareToPlay(m_sampleRate, m_blockSize);
            sample->transportSource.setPosition(0.0);
            sample->transportSource.start();
        }
    }
}

void GranularinfiniteAudioProcessor::stopPlayback(const juce::String& note)
{
    m_keyPressed = false;
    const int midi_note = CreateNoteToMidi[note];

    if (synthToggle)
    {
        synth.noteOff(1, midi_note, 127, false);
    }

    auto it = samples.find(note);
    if (it != samples.end())
    {
        auto& sample = it->second;

        std::thread([samplePtr = sample.get()] {
            if (samplePtr)
                samplePtr->transportSource.stop();
            }).detach();
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
