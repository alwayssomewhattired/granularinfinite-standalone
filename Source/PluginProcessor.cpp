/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    formatManager.registerBasicFormats();
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

    isPrepared = true;
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

void GranularinfiniteAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, 
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::AudioBuffer<float> tempBuffer;
    tempBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());

    for (auto& pair : samples)
    {
        tempBuffer.clear();
        juce::AudioSourceChannelInfo info(&tempBuffer, 0, buffer.getNumSamples());
        pair.second->transportSource.getNextAudioBlock(info);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.addFrom(ch, 0, tempBuffer, ch, 0, buffer.getNumSamples());
    }
}

void GranularinfiniteAudioProcessor::loadFile(const juce::File& file, const juce::String& noteName)
{
    if (auto* reader = formatManager.createReaderFor(file))
    {
        auto sample = std::make_unique<Sample>();
        sample->setSourceFromReader(reader);
        samples[noteName] = std::move(sample);
    }
}

void GranularinfiniteAudioProcessor::startPlayback(const juce::String& note)
{
    std::cout << "isPrepared=" << isPrepared
        << ", samples.size=" << samples.size() << "\n";
    for (auto& pair : samples)
        std::cout << "Sample: " << pair.first << ", readerSource=" << (pair.second->readerSource != nullptr) << "\n";

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
            sample->transportSource.setPosition(0.0);
            sample->transportSource.start();
        }
    }
}

void GranularinfiniteAudioProcessor::stopPlayback(const juce::String& note)
{
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
