#pragma once

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "GranularInfinite.h"
#include "KeyButton.h"
#include "SampleLabel.h"
#include "NoteLabel.h"
#include "ButtonPalette.h"
#include "DualThumbSlider.h"
#include "GrainPositionControl.h"
#include "KeyButtonMods.h"
#include <memory>
#include <algorithm>
#include "JuceHeader.h"
#include <juce_core/juce_core.h>





//==============================================================================
GranularInfinite::GranularInfinite(GranularinfiniteAudioProcessor& p, ButtonPalette& bp)
    :
    audioProcessor(p),
    buttonPalette(bp),
    grainLengthSlider(std::tuple<double, double, double>(512.0, 36000.0, 1.0)),
    grainSpacingLabel(buttonPalette.grainSpacingLabel),
    grainSpacingSlider(buttonPalette.grainSpacingSlider),
    grainAmountLabel(buttonPalette.grainAmountLabel),
    grainAmountSlider(buttonPalette.grainAmountSlider),
    grainAreaSlider(std::tuple<double, double, double>(0.0, 600.0, 1.0)),
    grainAreaLabel(buttonPalette.grainPositionLabel),
    grainLengthLabel(buttonPalette.grainLengthLabel),
    hanningToggleButton(&buttonPalette.hanningToggleButton)

{
    audioProcessor.addChangeListener(this);

    keyToNote = CreateKeyToNote(octave);
    setWantsKeyboardFocus(true);
    addKeyListener(this);
    setSize(1900, 1000);
    // find out if timer needed
    startTimerHz(60);

    // handler lambda initialization
    grainLengthSliderHandler();
    grainAreaSliderHandler();

    // intialize noteToSamples vector 
    const std::vector<std::string> notes = { "C0", "C#0", "D0", "D#0", "E0", "F0", "F#0", "G0", "G#0", "A0", "A#0", "B0",
        "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1",
        "C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2",
        "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A3", "A#3", "B3",
        "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4",
        "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5", "A#5", "B5",
        "C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6", "A6", "A#6", "B6",
        "C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7", "A7", "A#7", "B7",
        "C8", "C#8", "D8", "D#8", "E8", "F8" };


    for (std::string noteValue : notes)
    {
        noteToSample.set(noteValue, "");
    }
        // creates frequencyUpwardCompressors
    //m_keyButtonMods.setInterceptsMouseClicks(false, false);
    for (const auto& [k, v] : m_noteToFreq) {
        auto [sliderPtr, labelPtr] = buttonPalette.createFrequencyUpwardCompressor(v);

        sliderPtr->setLookAndFeel(&customLook);

        //addAndMakeVisible(*sliderPtr); 
        //addAndMakeVisible(*labelPtr);

        frequencyUpwardCompressorAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, 
            std::string("frequencyUpwardCompressorProminence") + k, *sliderPtr));

        m_keyButtonMods.addFrequencyUpwardCompressor(k, v, std::move(sliderPtr));
        m_keyButtonMods.addFrequencyUpwardCompressorLabel(k, std::move(labelPtr));
    }

    const std::string order = "awsedftgyhujkolp;'";
    int count = 0;
    int keyButtonX = 75;


    // IMPORTANT!
    // i have a bunch of stuff in this loop that only needs to be initialized once. get it out of there
    for (auto key : order)
    {
        if (keyToNote.find(key) != keyToNote.end()) count++;
        auto it = keyToNote.find(key);
        if (it != keyToNote.end())
        {



            //-------keyboard------//
            char myChar = it->first;
            juce::String myKeyName = juce::String::charToString(myChar);
            juce::String keyNameWithOctave = myKeyName + juce::String(count);
            juce::String myNoteName = it->second;

            // note-name
            auto* noteLabel = new NoteLabel(myNoteName);

            // piano-key
            auto* button = new KeyButton(myKeyName, myNoteName, keyNameWithOctave, keyButtonX);
            button->setColour(juce::TextButton::buttonColourId, juce::Colours::white);
            button->setColour(juce::TextButton::buttonOnColourId, juce::Colours::lightgrey);
            button->setColour(juce::TextButton::textColourOffId, juce::Colours::black);

            int buttonWidth = 60;
            int buttonHeight = 120;
            int labelHeight = 20;
            int spacing = 5;
            keyButtonX += buttonWidth + spacing + 35;

            // sample-name
            auto* sampleLabel = new SampleLabel("", myNoteName);
            button->setOnFileDropped([this, myNoteName, sampleLabel, button, myKeyName](std::map<juce::String, juce::Array<juce::File>>& noteToFiles, 
                const bool& isDir) {
                    const std::map<char, juce::String>& keyToNote = CreateKeyToNote(octave);

                    juce::String fullPath;
                    juce::String name;
                    juce::String refinedNote;

                    for (const auto& [k, v] : noteToFiles) {
                        for (const juce::File& audioFile : v) {
                            fullPath = audioFile.getFullPathName();
                            name = audioFile.getFileNameWithoutExtension();
                            button->setTrimmedFileName(name);
                            if (isDir) {
                                refinedNote = k;
                            }
                            else {
                                size_t lastUnderscore = name.toStdString().rfind('_');
                                if (lastUnderscore != std::string::npos) {
                                    refinedNote = name.substring(lastUnderscore + 1);
                                }
                                if (CreateNoteToMidi.find(refinedNote) == CreateNoteToMidi.end()) {
                                    refinedNote = myNoteName.dropLastCharacters(1) + juce::String(octave);
                                }
                            }

                            SampleLabel* matchingSampleLabel = sampleLabel;
                            matchingSampleLabel->setButtonText(name);
                            noteToSample.set(refinedNote, name);
                            juce::File file(fullPath);
                            juce::String refinedName = file.getFileNameWithoutExtension();
                            matchingSampleLabel->file = refinedName;
                            synthNote = refinedNote;

                            noteToFile[refinedNote] = std::make_unique<juce::File>(fullPath);
                            GranularinfiniteAudioProcessor::Sample* samplePtr = audioProcessor.loadFile(file, refinedNote, "false");

                             //i can't see my waveform button anymore
                            buttonPalette.addWaveformButton(refinedName, myKeyName, refinedNote,
                                [this, refinedNote, refinedName, samplePtr](juce::TextButton& button)
                                {
                                    juce::String& state = buttonPalette.waveformState;
                                    if (button.getToggleState())
                                    {
                                        if (state.isNotEmpty())
                                        {
                                            if (auto it = buttonPalette.waveformButtons.find(state); it != buttonPalette.waveformButtons.end())
                                            {
                                                it->second->waveformButton->setToggleState(false, juce::dontSendNotification);
                                            }
                                        }
                                        state = refinedName;
                                        m_waveformDisplay.setBuffer(audioProcessor.getSampleBuffer(refinedNote));
                                        m_waveformDisplay.setSample(samplePtr);

                                        m_waveformDisplay.setPlayheadPosition();
                                        button.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
                                    }
                                    else {
                                        state = juce::String();
                                        m_waveformDisplay.clear();
                                    }
                                });
                        }
                    }
                    
                    resized();
                    sampleRefresh(button);
                });
            //------------//

            // control-buttons

            //componentButton = buttonPalette.componentButton;
            componentButton.setButtonText(buttonPalette.componentButton.getButtonText());
            componentButton.setColour(
                juce::TextButton::buttonColourId,
                buttonPalette.componentButton.findColour(juce::TextButton::buttonColourId)
            );

            juce::TextButton& octaveIncrement = buttonPalette.incrementButton;
            juce::TextButton& octaveDecrement = buttonPalette.decrementButton;


            // custom looknfeel
            grainSpacingSlider.setLookAndFeel(&customLook);
            grainAmountSlider.setLookAndFeel(&customLook);
            grainLengthSlider.setLookAndFeel(&diySlider);
            grainAreaSlider.setLookAndFeel(&diySlider);


            addAndMakeVisible(buttonPalette);
            addAndMakeVisible(componentButton);
            addAndMakeVisible(octaveIncrement);
            addAndMakeVisible(octaveDecrement);
            addAndMakeVisible(grainSpacingLabel);
            addAndMakeVisible(grainSpacingSlider);
            addAndMakeVisible(grainAmountLabel);
            addAndMakeVisible(grainAmountSlider);
            addAndMakeVisible(grainLengthLabel);
            addAndMakeVisible(grainLengthSlider);
            addAndMakeVisible(grainAreaLabel);
            addAndMakeVisible(grainAreaSlider);
            addAndMakeVisible(noteLabel);
            addAndMakeVisible(button);
            addAndMakeVisible(sampleLabel);
            addAndMakeVisible(buttonPalette.synthToggleButton);
            addAndMakeVisible(buttonPalette.hanningToggleButton);
            addAndMakeVisible(buttonPalette.compressor.thresholdStruct.thresholdSlider);
            addAndMakeVisible(buttonPalette.compressor.thresholdStruct.inc);
            addAndMakeVisible(buttonPalette.compressor.thresholdStruct.dec);
            addAndMakeVisible(buttonPalette.compressor.ratioSlider);
            addAndMakeVisible(buttonPalette.compressor.attackCoeffSlider);
            addAndMakeVisible(buttonPalette.compressor.releaseCoeffSlider);
            addAndMakeVisible(buttonPalette.compressor.gainSlider);
            addAndMakeVisible(buttonPalette.sampleCompressorLabel);
            addAndMakeVisible(buttonPalette.compressor.thresholdStruct.thresholdSliderLabel);
            addAndMakeVisible(buttonPalette.compressor.ratioSliderLabel);
            addAndMakeVisible(buttonPalette.compressor.attackCoeffSliderLabel);
            addAndMakeVisible(buttonPalette.compressor.releaseCoeffSliderLabel);
            addAndMakeVisible(buttonPalette.compressor.gainSliderLabel);
            addAndMakeVisible(m_waveformDisplay);
            addAndMakeVisible(m_keyButtonMods);



            buttonPalette.onWaveformButtonAdded = [this]() {
                resized();
                };
            
            componentButton.onClick = [this]() {
                    // (basically means 'if set'...)
                    if (onComponentButtonClicked)
                        onComponentButtonClicked();
                };

            octaveUp(octaveIncrement);
            octaveDown(octaveDecrement);

            grainSpacingAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "grainSpacing", grainSpacingSlider);
            grainAmountAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "grainAmount", grainAmountSlider);
            //grainAreaAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "grainArea", grainAreaSlider);

            hanningToggleAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "hanningToggle", *hanningToggleButton);

            compressorThresholdAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "compressorThreshold", 
                buttonPalette.compressor.thresholdStruct.thresholdSlider);
            compressorRatioAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "compressorRatio", 
                buttonPalette.compressor.ratioSlider);
            compressorAttackCoeffAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "compressorAttackCoeff", 
                buttonPalette.compressor.attackCoeffSlider);
            compressorReleaseCoeffAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "compressorReleaseCoeff", 
                buttonPalette.compressor.releaseCoeffSlider);
            compressorGainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "compressorGain", 
                buttonPalette.compressor.gainSlider);


            synthToggleHandler(buttonPalette.synthToggleButton);
            sampleLabelHandler(*sampleLabel);

            noteLabels.add(noteLabel);
            keyButtons.add(button);
            sampleLabels.add(sampleLabel);
                    }
    }
    resized();

}

GranularInfinite::~GranularInfinite()
{
    grainSpacingSlider.setLookAndFeel(nullptr);
    grainAmountSlider.setLookAndFeel(nullptr);
    grainLengthSlider.setLookAndFeel(nullptr);
    grainAreaSlider.setLookAndFeel(nullptr);
    //for (auto& [k, v] : frequencyUpwardCompressors) {
    //    v.slider->setLookAndFeel(nullptr);
    //}
}

//==============================================================================

// this lets the audioprocessor trigger a change on the editor
void GranularInfinite::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    // Example: check which object sent the change
    if (source == &audioProcessor)
    {
        // Respond to parameter change, or update UI
        //grainPositionSlider.setRange(0.0, (double)audioProcessor.getMaxFileSize(), 1.0);
    }
}

//void GranularInfinite::componentButtonHandler()
//{
//    auto& button = buttonPalette.componentButton;
//    button.onClick = [this, &button]() {
//        if (button.getToggleState())
//        {
//            if (onComponentButtonClicked)
//                onComponentButtonClicked();
//        }
//    };
//}

bool GranularInfinite::keyPressed(const juce::KeyPress& key,
    Component* originatingComponent)
{
    if (currentlyPressedKeys.find(static_cast<char>(key.getTextCharacter())) != currentlyPressedKeys.end()) {
        return false;
    }
    const std::string order = "awsedftgyhujkolp;'";
    char char_key = static_cast<char>(key.getTextCharacter());
    auto it = keyToNote.find(char_key);
    if (it != keyToNote.end())
    {
        const juce::String& noteName = it->second;

        auto it2 = currentlyPressedKeys.find(char_key);
        size_t index = order.find(char_key);
        if (index != std::string::npos)
        {
            auto* button = keyButtons[(int)index];
            if (audioProcessor.synthToggle)
            {
                if (it2 == currentlyPressedKeys.end())
                {
                    const int midi_note = CreateNoteToMidi[noteName];
                    juce::MidiMessage m = juce::MidiMessage::noteOn(1, midi_note, (juce::uint8)127);
                    audioProcessor.addMidiEvent(m);
                    currentlyPressedKeys.insert(char_key);

                    button->setColour(juce::TextButton::buttonColourId, juce::Colours::grey);

                    return true;
                }
                else {
                    return false;
                }
            }
        }
        currentlyPressedKeys.insert(char_key);

        if (index != std::string::npos)
        {
            auto* button = keyButtons[(int)index];

            button->setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
            button->repaint();
            if (auto* sampleName = noteToSample.getValue(noteName))
            {
                {

                    if (sampleName->isNotEmpty())
                    {
                        if (noteName.isNotEmpty()) {
                            if (audioProcessor.grainAll)
                                audioProcessor.updateCurrentSamples(noteName, false);
                        }
                        audioProcessor.startPlayback(noteName);
                    }
                    return true;
            }
        }
            else {
                return false;
            }
        }
    }
    return false;
}

bool GranularInfinite::keyStateChanged(bool isKeyDown,
    Component* originatingComponent)
{
    const std::string order = "awsedftgyhujkolp;'";

    if (!isKeyDown)
    {
        for (auto it = currentlyPressedKeys.begin(); it != currentlyPressedKeys.end();)
        {
            char keyChar = *it;
            if (!juce::KeyPress::isKeyCurrentlyDown(keyChar))
            {
                size_t index = order.find(keyChar);
                if (index != std::string::npos)
                {
                    if (keyChar == 'w' || keyChar == 'e' || keyChar == 't' || keyChar == 'y'
                        || keyChar == 'u' || keyChar == 'o' || keyChar == 'p')
                    {
                        keyButtons[(int)index]->setColour(juce::TextButton::buttonColourId,
                            juce::Colours::black);
                        keyButtons[(int)index]->repaint();
                    }
                    else {
                        keyButtons[(int)index]->setColour(juce::TextButton::buttonColourId,
                            juce::Colours::white);
                        keyButtons[(int)index]->repaint();
                    }
                }
                juce::String noteName = keyToNote[keyChar];
                it = currentlyPressedKeys.erase(it);

                auto sampleNameIt = keyToNote.find(keyChar);
                if (sampleNameIt != keyToNote.end()) {
                    if (auto* sampleName = noteToSample.getValue(sampleNameIt->second)) {
                        if (audioProcessor.grainAll)
                            audioProcessor.updateCurrentSamples(noteName, true);
                    }
                }
                audioProcessor.stopPlayback(noteName);
            }
            else
            {
                ++it;
            }
        }
    }
    return false;
}

// *TO-DO*
// - finish the 'sampleRefresh' function (make it work almost like 'octaveUp'.

// - use this after dropping file/folder.
// - lazily fixes SampleLabel issue.
void GranularInfinite::sampleRefresh(KeyButton* button) {

    auto& keyToNote = CreateKeyToNote(octave);
    const std::string order = "awsedftgyhujkolp;'";

    for (size_t i = 0; i < order.size(); i++)
    {
        auto it = keyToNote.find(order[i]);
        if (it != keyToNote.end())
        {
            auto* sample = noteToSample.getValue(it->second);
            if (!sample)
            {
                sampleLabels[i]->setButtonText("");
            }
            else {
                juce::String raw = *sample;
                sampleLabels[i]->setButtonText(raw);
            }
        }
    }
}

void GranularInfinite::octaveUp(juce::TextButton& button)
{
    button.onClick = [this] {
        octave = std::clamp(octave + 1, 0, 8);
        keyToNote = CreateKeyToNote(octave);
        const std::string order = "awsedftgyhujkolp;'";

        for (size_t i = 0; i < order.size(); i++)
        {
            auto it = keyToNote.find(order[i]);
            if (it != keyToNote.end())
            {
                auto* sample = noteToSample.getValue(it->second);
                if (!sample)
                {
                    sampleLabels[i]->setButtonText("");
                }
                else {
                    juce::String raw = *sample;
                    sampleLabels[i]->setButtonText(raw);
                }
                noteLabels[i]->setText(it->second, juce::dontSendNotification);
            }
        }
        resized();
        };

}

void GranularInfinite::octaveDown(juce::TextButton& button)
{
    button.onClick = [this] {
        octave = std::clamp(octave - 1, 0, 8);
        keyToNote = CreateKeyToNote(octave);
        const std::string order = "awsedftgyhujkolp;'";
        for (size_t i = 0; i < order.size(); i++)
        {
            auto it = keyToNote.find(order[i]);
            if (it != keyToNote.end())
            {
                auto* sample = noteToSample.getValue(it->second);
                if (!sample) {
                    sampleLabels[i]->setButtonText("");
                }
                else {
                    juce::String raw = *sample;
                    sampleLabels[i]->setButtonText(raw);
                }
                noteLabels[i]->setText(it->second, juce::dontSendNotification);
            }
        }
        resized();

        };

}


void GranularInfinite::grainLengthSliderHandler()
{
    grainLengthSlider.onRangeChange = [this]()
        {
            auto* minParam = audioProcessor.apvts.getParameter("grainMinLength");
            if (auto* floatParam = dynamic_cast<juce::RangedAudioParameter*>(minParam))
            {
                float normalized = floatParam->convertTo0to1((float)grainLengthSlider.getMinValue());
                floatParam->setValueNotifyingHost(normalized);
            }

            auto* maxParam = audioProcessor.apvts.getParameter("grainMaxLength");
            if (auto* floatParam = dynamic_cast<juce::RangedAudioParameter*>(maxParam))
            {
                float normalized = floatParam->convertTo0to1((float)grainLengthSlider.getMaxValue());
                floatParam->setValueNotifyingHost(normalized);
            }
        };
}

void GranularInfinite::grainAreaSliderHandler()
{
    grainAreaSlider.onRangeChange = [this]()
        {
            float grainArea = grainAreaSlider.getValue();
            m_waveformDisplay.setGrainArea(grainArea);
            
            auto* minParam = audioProcessor.apvts.getParameter("grainMinArea");
            if (auto* floatParam = dynamic_cast<juce::RangedAudioParameter*>(minParam)) {
                float normalized = floatParam->convertTo0to1((float)grainAreaSlider.getMinValue());
                floatParam->setValueNotifyingHost(normalized);
            }

            auto* maxParam = audioProcessor.apvts.getParameter("grainMaxArea");
            if (auto* floatParam = dynamic_cast<juce::RangedAudioParameter*>(maxParam)) {
                float normalized = floatParam->convertTo0to1((float)grainAreaSlider.getMaxValue());
                floatParam->setValueNotifyingHost(normalized);
            }
        };
}


void GranularInfinite::timerCallback()
{
    playheadPositionHandler();
}

void GranularInfinite::playheadPositionHandler()
{
    const juce::AudioBuffer<float> buffer = m_waveformDisplay.getBuffer();
    m_waveformDisplay.setPlayheadPosition();
}

void GranularInfinite::sampleLabelHandler(SampleLabel& button)
{
    // make it so only the latest value is reflected onto the button change.
    button.onClick = [this, &button] {
        if (auto* note = noteToSample.getKey(button.file))
        {
            std::string realStr = note->toStdString();
            juce::String jStr = realStr;
            auto it = noteToFile.find(realStr);

            if (it != noteToFile.end() && it->second)
            {

                juce::File& fullFile = *(it->second);
                juce::String tmpFileName = fullFile.getFileNameWithoutExtension();
                if (currentlyPressedSample != "none")
                {
                    auto sample_it = std::find_if(sampleLabels.begin(), sampleLabels.end(), [this](SampleLabel* label)
                        { return label->file == currentlyPressedSample; });
                    if (sample_it != sampleLabels.end())
                    {
                        juce::String sample_value = (*sample_it)->file;
                        (*sample_it)->setToggleState(false, juce::dontSendNotification);
                        currentlyPressedSample = tmpFileName;
                        if (sample_value == currentlyPressedSample) {
                            currentlyPressedSample = "none";
                        }
                    }

                }
                else {
                    currentlyPressedSample = tmpFileName;
                }
                // no need to pass in currentlyPressedSample
                audioProcessor.loadFile(fullFile, jStr, currentlyPressedSample);
            }
            else {
                std::cout << "Error with file my son\n";
                return;
            }

            bool isToggled = button.getToggleState();
            if (isToggled)
            {
                audioProcessor.synthToggle = true;
            }
            else {
                audioProcessor.synthToggle = false;
            }
        }
        else {
            std::cout << "noteToSample fail\n";
        }
        };
}

void GranularInfinite::synthToggleHandler(juce::TextButton& button)
{
    button.onClick = [this, &button] {
        bool isToggled = button.getToggleState();

        if (isToggled)
        {
            audioProcessor.grainAll = true;
            button.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
        }
        else {
            audioProcessor.grainAll = false;
            button.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
        }
        };
}


//==============================================================================


void GranularInfinite::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void GranularInfinite::resized()
// when you get the time, for the love of god refactor using flexbox
{


    int x = 300;
    int y = 100;
    int buttonWidth = 60;
    int buttonHeight = 120;
    int labelHeight = 20;
    int spacing = 5;


    buttonPalette.setBounds(getLocalBounds());

    m_keyButtonMods.setBounds(0, 100, 4000, 300);

    buttonPalette.componentButton.setBounds(1600, y + 400, 80, 80);
    componentButton.setBounds(1600, y + 400, 80, 80);

    buttonPalette.sampleCompressorLabel.setBounds(1300, y + 300, 80, 100);
    buttonPalette.compressor.thresholdStruct.thresholdSlider.setBounds(1300, y + 400, 80, 100);
    buttonPalette.compressor.thresholdStruct.inc.setBounds(1375, y + 440, 20, 20);
    buttonPalette.compressor.thresholdStruct.dec.setBounds(1375, y + 460, 20, 20);
    buttonPalette.compressor.ratioSlider.setBounds(1300, y + 500, 80, 100);
    buttonPalette.compressor.attackCoeffSlider.setBounds(1300, y + 600, 80, 100);
    buttonPalette.compressor.releaseCoeffSlider.setBounds(1300, y + 700, 80, 100);
    buttonPalette.compressor.gainSlider.setBounds(1300, y + 800, 80, 100);
    buttonPalette.compressor.thresholdStruct.thresholdSliderLabel.setBounds(1200, y + 400, 80, 100);
    buttonPalette.compressor.ratioSliderLabel.setBounds(1200, y + 500, 80, 100);
    buttonPalette.compressor.attackCoeffSliderLabel.setBounds(1200, y + 600, 80, 100);
    buttonPalette.compressor.releaseCoeffSliderLabel.setBounds(1200, y + 700, 80, 100);
    buttonPalette.compressor.gainSliderLabel.setBounds(1200, y + 800, 80, 100);

    buttonPalette.decrementButton.setBounds(x - 200, y + 350, buttonWidth, buttonHeight);
    buttonPalette.incrementButton.setBounds(x - 100, y + 350, buttonWidth, buttonHeight);

    buttonPalette.synthToggleButton.setBounds(x, y + 350, buttonWidth, buttonHeight);

    buttonPalette.hanningToggleButton.setBounds(x + 100, y + 350, buttonWidth, buttonHeight);

    m_waveformDisplay.setBounds(650, 450, 600, 200);

    juce::FlexBox outer;
    juce::FlexBox inner1;
    juce::FlexBox inner2;
    inner1.flexDirection = juce::FlexBox::Direction::column;
    inner1.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    inner1.alignItems = juce::FlexBox::AlignItems::center;

    inner2.flexDirection = juce::FlexBox::Direction::column;
    inner2.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    inner2.alignItems = juce::FlexBox::AlignItems::center;

    juce::Rectangle<int> controlBounds(0, 520, 400, 600);
    juce::Rectangle<int> controlBounds2(300, 520, 400, 600);


    juce::FlexItem f_grainSpacingLabel(grainSpacingLabel);
    inner1.items.add(
        f_grainSpacingLabel
        .withHeight(30.0f)
        .withWidth(controlBounds.getWidth())
    );

    juce::FlexItem f_grainSpacingSlider(grainSpacingSlider);
    inner1.items.add(
        f_grainSpacingSlider
        .withHeight(100.0f)
        .withWidth(200.0f)
    );

    juce::FlexItem f_grainAmountLabel(grainAmountLabel);
    inner1.items.add(
        f_grainAmountLabel
        .withHeight(30.0f)
        .withWidth(controlBounds.getWidth())
    );

    juce::FlexItem f_grainAmountSlider(grainAmountSlider);
    inner1.items.add(
        f_grainAmountSlider
        .withHeight(100.0f)
        .withWidth(200.0f)
    );

    juce::FlexItem f_grainLengthLabel(grainLengthLabel);
    inner2.items.add(
        f_grainLengthLabel
        .withHeight(30.0f)
        .withWidth(controlBounds.getWidth())
    );

    juce::FlexItem f_grainLengthSlider(grainLengthSlider);
    inner2.items.add(
        f_grainLengthSlider
        .withHeight(100.0f)
        .withWidth(200.0f)
    );

    juce::FlexItem f_grainPositionLabel(grainAreaLabel);
    inner2.items.add(
        f_grainPositionLabel
        .withHeight(30.0f)
        .withWidth(controlBounds.getWidth())
    );

    juce::FlexItem f_grainAreaSlider(grainAreaSlider);
    inner2.items.add(
        f_grainAreaSlider
        .withHeight(100.f)
        .withWidth(200.0f)
    );

    outer.items.add(
        juce::FlexItem(inner1)
        .withMargin({ 100, 0, 0, 0 })
    );
    outer.items.add(
        juce::FlexItem(inner2)
        .withMargin({ 100, 0, 0, 0 })
    );
    outer.performLayout(controlBounds);

    auto inner1Area = controlBounds.withTrimmedTop(100);
    inner1.performLayout(inner1Area.toFloat());
    auto inner2Area = controlBounds2.withTrimmedTop(100);
    inner2.performLayout(inner2Area.toFloat());

    int keyControlX = 75;
    m_keyButtonMods.drawUpwardCompressors(noteRange, octave, keyControlX, y, buttonWidth, buttonHeight, spacing, buttonPalette);


    int keyButtonX = 75;
    for (int i = 0; i < keyButtons.size(); ++i)
    {
        //auto& keyButton = keyButtons[i];
        noteLabels[i]->setBounds(keyButtonX, y - 50, buttonWidth, labelHeight);
        if (i == 1 || i == 3 || i == 6 || i == 8 || i == 10 || i == 13 || i == 15)
        {
            keyButtons[i]->setBounds(keyButtonX + 2, y - 10, buttonWidth - 5, buttonHeight - 25);
            keyButtons[i]->setColour(juce::TextButton::buttonColourId, juce::Colours::black);
            keyButtons[i]->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
            keyButtons[i]->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            keyButtons[i]->setColour(juce::ComboBox::outlineColourId, juce::Colours::white);
        }
        else {
            keyButtons[i]->setBounds(keyButtonX, y, buttonWidth, buttonHeight);
        }
        sampleLabels[i]->setBounds(keyButtonX, y + buttonHeight, buttonWidth, labelHeight);
        for (auto& [k, v] : buttonPalette.waveformButtons) {
            if (keyButtons[i]->getNoteName() == v->noteName) {
                const auto& waveformButton = v->waveformButton;
                addAndMakeVisible(*waveformButton);
                waveformButton->setBounds(keyButtons[i]->getPosition(), y + buttonHeight + labelHeight + 10, buttonWidth, labelHeight);
            }
        }
        const auto& fileName = keyButtons[i]->getTrimmedFileName();

        keyButtonX += buttonWidth + spacing + 35;   // move to next key
    }

}

