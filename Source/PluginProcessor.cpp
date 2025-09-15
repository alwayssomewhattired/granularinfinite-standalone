/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "OpenConsole.h"
#include <algorithm>

//==============================================================================
GranularinfiniteAudioProcessor::GranularinfiniteAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
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
    std::cout << "PREPARE TO PAY" << std::endl;
    for (auto& pair : samples)
    {
        pair.second->transportSource.prepareToPlay(samplesPerBlock, sampleRate);
    }

    //for (auto& pair : samples)
    //{
    //    pair.second->tempBuffer.setSize(pair.second->transportSource.getTotalNumOutputChannels(),
    //        samplesPerBlock + 32, false, false, true);
    //    pair.second->bufferSize = pair.second->tempBuffer.getNumSamples();
    //    pair.second->bufferPos = 0;
    //}
    std::cout << sampleRate << "\n";
    std::cout << samplesPerBlock << "\n";
    synth.setCurrentPlaybackSampleRate(sampleRate);

    tempBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock, false, false, true);
    tempBuffer.clear();

    isPrepared = true;
}

void GranularinfiniteAudioProcessor::releaseResources()
{
    std::cout << "REALEASE RESOURCES\n";
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

//void GranularinfiniteAudioProcessor::injectNoteOn(juce::MidiBuffer& midiMessages, 
//    const int& midiNote)
//{
//    // i think this juce::midimessage object needs to be timestamped
//    juce::MidiMessage m = juce::MidiMessage::noteOn(1, midiNote, (juce::uint8)127);
//    midiMessages.addEvent(m, 0);
//}
//
//void GranularinfiniteAudioProcessor::injectNoteOff(juce::MidiBuffer& midiMessages,
//    const int& midiNote)
//{
//    juce::MidiMessage m = juce::MidiMessage::noteOff(1, midiNote);
//    midiMessages.addEvent(m, 0);
//}

void GranularinfiniteAudioProcessor::addMidiEvent(const juce::MidiMessage& m)
{
    const std::lock_guard<std::mutex> lock(midiMutex);
    midiFifo.addEvent(m, 0);
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



    if (synthToggle)
    {
        {
            const std::lock_guard<std::mutex> lock(midiMutex);
            midiMessages.addEvents(midiFifo, 0, numSamples, 0);
            midiFifo.clear();
        }

        // debug: print incoming MIDI descriptions (small volume)
        for (auto meta : midiMessages)
            std::cout << "MIDI: " << meta.getMessage().getDescription() << "\n";

        // render synth into cleared buffer
        buffer.clear();
        synth.renderNextBlock(buffer, midiMessages, 0, numSamples);
        return;
    }

    // TRANSPORT PATH:
    // Reuse tempBuffer — clear it once per sample
    buffer.clear();

    for (auto& pair : samples)
    {
        auto& sample = pair.second;

        // debug: check state
        //std::cout << "Sample '" << pair.first << "' isPlaying=" << sample->transportSource.isPlaying()
        //    << " pos=" << sample->transportSource.getCurrentPosition() << "\n";

        // clear and make sure channel counts are OK
        tempBuffer.clear();

        // getNextAudioBlock will write into tempBuffer
        juce::AudioSourceChannelInfo info(&tempBuffer, 0, numSamples);

        // Protect calls with try/catch in debug builds if desired (JUCE exceptions rarely used)
        sample->transportSource.getNextAudioBlock(info);

        // mix into main buffer safely (avoid out-of-bounds if tempBuffer channels < output channels)
        const int srcChans = tempBuffer.getNumChannels();
        const int chansToMix = std::min(outCh, srcChans);

        for (int ch = 0; ch < chansToMix; ++ch)
            buffer.addFrom(ch, 0, tempBuffer, ch, 0, numSamples);

        // if src is mono and outCh stereo, copy mono into right channel as well
        if (srcChans == 1 && outCh >= 2)
            buffer.addFrom(1, 0, tempBuffer, 0, 0, numSamples);
    }
}


//void GranularinfiniteAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, 
//    juce::MidiBuffer& midiMessages)
//{
//    juce::ScopedNoDenormals noDenormals;
//    auto totalNumInputChannels  = getTotalNumInputChannels();
//    auto totalNumOutputChannels = getTotalNumOutputChannels();
//
//    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
//        buffer.clear (i, 0, buffer.getNumSamples());
//    if (synthToggle)
//    {
//        for (auto metadata : midiMessages)
//        {
//            std::cout << "WHY\n";
//            std::cout << metadata.getMessage().getDescription() << "\n";
//        }
//        const std::lock_guard<std::mutex> lock(midiMutex);
//        midiMessages.addEvents(midiFifo, 0, buffer.getNumSamples(), 0);
//        midiFifo.clear();
//        synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
//        return;
//    }
//    tempBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());
//
//    for (auto& pair : samples)
//    {
//        std::cout << "Playing? " << pair.second->transportSource.isPlaying()
//            << " pos=" << pair.second->transportSource.getCurrentPosition() << "\n";
//
//        tempBuffer.clear();
//        juce::AudioSourceChannelInfo info(&tempBuffer, 0, buffer.getNumSamples());
//        pair.second->transportSource.getNextAudioBlock(info);
//
//        int numChans = std::min(buffer.getNumSamples(), tempBuffer.getNumChannels());
//        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
//            buffer.addFrom(ch, 0, tempBuffer, ch, 0, buffer.getNumSamples());
//    }
//}

void GranularinfiniteAudioProcessor::loadFile(const juce::File& file, const juce::String& noteName)
{
    std::cout << "ONFILE" << std::endl;
    if (auto* reader = formatManager.createReaderFor(file))
    {
        if (!synthToggle)
        {
            auto sample = std::make_unique<Sample>();
            sample->setSourceFromReader(reader);
            //sample->transportSource.prepareToPlay(48000.0, 576);
            samples[noteName] = std::move(sample);
        }
        else {
            int rootNote = CreateNoteToMidi.at(noteName);
            juce::BigInteger allNotes;
            allNotes.setRange(12, 113, true);
            const int midi_note = CreateNoteToMidi[noteName];
            auto sound = new juce::SamplerSound(noteName.toStdString(), *reader, allNotes,
                rootNote, 0.0, 0.0, 10.0);
            synth.addSound(sound);
        }

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
    //std::cout << "isPrepared=" << isPrepared
    //    << ", samples.size=" << samples.size() << "\n";
    //for (auto& pair : samples)
        //std::cout << "Sample: " << pair.first << ", readerSource=" << (pair.second->readerSource != nullptr) << "\n";
    if (!isPrepared)
    {
        std::cout << "not ready\n";
        return;
    }
    auto it = samples.find(note);
    if (it != samples.end())
    {
        auto& sample = it->second;
        if (!sample->transportSource.isPlaying())
        {
                prepareToPlay(48000.0, 576);
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
