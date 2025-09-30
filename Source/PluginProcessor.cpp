

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "OpenConsole.h"
#include <juce_dsp/juce_dsp.h>
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
#endif
{
    minGrainLengthPtr = apvts.getRawParameterValue("grainMinLength");
    jassert(minGrainLengthPtr != nullptr);
    maxGrainLengthPtr = apvts.getRawParameterValue("grainMaxLength");
    jassert(maxGrainLengthPtr != nullptr);

    openConsole();
    formatManager.registerBasicFormats();
    
    for (int i = 0; i < 16; ++i)
        synth.addVoice(new juce::SamplerVoice());
}

GranularinfiniteAudioProcessor::~GranularinfiniteAudioProcessor()
{
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
        512
    ));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "grainMinLength",
        "GrainMinLength",
        128,
        48000,
        36000
    ));

    return { params.begin(), params.end() };
}


void GranularinfiniteAudioProcessor::addMidiEvent(const juce::MidiMessage& m)
{
    const std::lock_guard<std::mutex> lock(midiMutex);
    midiFifo.addEvent(m, 0);
}


void GranularinfiniteAudioProcessor::spawnGrain()
{
    if (circularBuffer.getNumSamples() == 0)
        return;

    if ((int)grains.size() >= grainAmount)
        return;

    Grain g;

    g.startSample = juce::Random::getSystemRandom().nextInt(circularBuffer.getNumSamples());

    g.length = juce::Random::getSystemRandom().nextInt(maxGrainLength - minGrainLength + 1) + minGrainLength;
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
    // circularBuffer filling
    buffer.clear();
    minGrainLength = *minGrainLengthPtr;
    maxGrainLength = *maxGrainLengthPtr;
    for (auto& pair : samples)
    {
        auto& sample = pair.second;

        tempBuffer.clear();
        juce::AudioSourceChannelInfo info(&tempBuffer, 0, numSamples);
        sample->transportSource.getNextAudioBlock(info);

        for (int ch = 0; ch < tempBuffer.getNumChannels(); ++ch)
        {
            int firstPart = std::min(numSamples, circularBuffer.getNumSamples() - circularWritePos);
            int secondPart = numSamples - firstPart;

            int destCh = juce::jmin(ch, circularBuffer.getNumChannels() - 1);
            int srcCh = juce::jmin(ch, tempBuffer.getNumChannels() - 1);

            circularBuffer.copyFrom(destCh, circularWritePos, tempBuffer, srcCh, 0, firstPart);

            //copy second part if wrap-around
            if (secondPart > 0)
                circularBuffer.copyFrom(destCh, 0, tempBuffer, srcCh, firstPart, secondPart);
        }
        circularWritePos = (circularWritePos + numSamples) % circularBuffer.getNumSamples();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float out = 0.0f;

            for (auto it = grains.begin(); it != grains.end(); )
            {
                Grain& g = *it;
                if (g.position < g.length)
                {
                    int readIndex = (g.startSample + (int)(g.position * g.pitchRatio)) % circularBuffer.getNumSamples();
                    int idx = (g.position * hannWindow.size()) / g.length;
                    float env = hannWindow[idx];
                    out += circularBuffer.getSample(0, readIndex) * env;
                    ++g.position;
                    ++it;
                }
                else {
                    it = grains.erase(it);
                }
            }
            for (int ch = 0; ch < outCh; ++ch)
            {
                buffer.addSample(ch, sample, out);
            }
        }

        grainCounter += numSamples;
        if (grainCounter >= grainSpacing)
        {
            grainCounter = 0;
            spawnGrain();
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
    //
    // SYNTH PATH
    //
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
        processGranularPath(buffer, outCh, numSamples);
        return;
    }
    else {
        processSamplerPath(buffer, outCh, numSamples);
        return;
    }
}

void GranularinfiniteAudioProcessor::loadFile(const juce::File& file, const juce::String& noteName, std::optional<juce::String> fileName)
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
        samples[noteName] = std::move(sample);

    }
}

void GranularinfiniteAudioProcessor::startPlayback(const juce::String& note)
{
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
    const int midi_note = CreateNoteToMidi[note];

    if (synthToggle)
    {
        synth.noteOff(1, midi_note, 127, false);
    }
    auto it = samples.find(note);
    if (it != samples.end())
    {
        auto& sample = it->second;
        sample->transportSource.stop();
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
