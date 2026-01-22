#pragma once

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "GranularInfinite.h"
#include "components/KeyButton.h"
#include "components/SampleLabel.h"
#include "NoteLabel.h"
#include "components/ButtonPalette.h"
#include "components/DualThumbSlider.h"
#include "components/CompressorWaveformComponent.h"
#include "components/GrainPositionControl.h"
#include "components/KeyButtonMods.h"
#include "Constants.h"
#include <memory>
#include <algorithm>
#include "JuceHeader.h"
#include <juce_core/juce_core.h>





//==============================================================================
GranularInfinite::GranularInfinite(GranularinfiniteAudioProcessor& p, ButtonPalette& bp)
    :
    audioProcessor(p),
    buttonPalette(bp),
    m_apvts(p.apvts),
    grainLengthSlider(std::tuple<double, double, double>(512.0, 36000.0, 1.0)),
    grainDensityLabel(buttonPalette.grainDensityLabel),
    grainDensitySlider(buttonPalette.grainDensitySlider),
    grainAmountLabel(buttonPalette.grainAmountLabel),
    grainAmountSlider(buttonPalette.grainAmountSlider),
    chunkCrossfadeLabel(buttonPalette.chunkCrossfadeLabel),
    chunkCrossfadeSlider(buttonPalette.chunkCrossfadeSlider),
    grainAreaSlider(std::tuple<double, double, double>(0.0, 100.0, 100.0)),
    grainAreaLabel(buttonPalette.grainAreaLabel),
    grainLengthLabel(buttonPalette.grainLengthLabel),
    hanningToggleButton(&buttonPalette.hanningToggleButton),
    m_keyButtonMods(m_waveformDisplay, m_scrollableList)
{

    audioProcessor.addChangeListener(this);

    keyToNote = CreateKeyToNote(*m_octave);

    setWantsKeyboardFocus(true);
    addKeyListener(this);
    //std::cout << "1\n";
    //setSize(1900, 1000);
    //std::cout << "2\n";

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


    int keyButtonX = 75;

    std::vector<juce::String> keys;
    for (auto key : Constants::KEY_ORDER) {
        keys.emplace_back(juce::String::charToString(key));
    }
    m_keyButton = std::make_shared<KeyButtons>(keys, *m_octave);

    for (const auto& [k, v] : m_noteToFreq) {
        auto [sliderPtr, labelPtr] = buttonPalette.createFrequencyUpwardCompressor(v);

        sliderPtr->setLookAndFeel(&customLook);

        //addAndMakeVisible(*sliderPtr); 
        //addAndMakeVisible(*labelPtr);

        frequencyUpwardCompressorAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, 
            std::string("frequencyUpwardCompressorProminence") + k, *sliderPtr));

        m_keyButtonMods.addFrequencyUpwardCompressor(k, v, std::move(sliderPtr));
        m_keyButtonMods.addFrequencyUpwardCompressorLabel(k, std::move(labelPtr));
        auto octavePtr = m_octave;
        m_keyButtonMods.setOctave(std::move(octavePtr));
        std::shared_ptr<KeyButtons> m_keyButtonPtr = m_keyButton;
        m_keyButtonMods.setKeyButtons(m_keyButtonPtr);
    }

    // this is a loop for all keybuttons
    int firstCount = 0;
    for (const auto note : allNotes()) {

            //-------keyboard------//
            juce::String myKeyName = juce::String::charToString(Constants::KEY_ORDER[firstCount % Constants::KEY_ORDER.size()]);
            juce::String myNoteName (note.data(), note.size());

            // note-name
            auto* noteLabel = new NoteLabel(myNoteName);

            int buttonWidth = 60;
            int buttonHeight = 120;
            int labelHeight = 20;
            int spacing = 5;
            keyButtonX += buttonWidth + spacing + 35;

            // sample-name
            auto* sampleLabel = new SampleLabel("", myNoteName);

            //juce::String refinedKeyOctave = juce::String(key) + (m_octave * Constants::MAX_OCTAVE);
            auto* button = m_keyButton->getKeyButton(firstCount);
            juce::Component::SafePointer<KeyButton> safeButton(button);
            button->setOnFileDropped([this, myNoteName, sampleLabel, safeButton, myKeyName](std::map<juce::String, std::map<juce::File, bool>>& noteToFiles,
                const bool& isDir) {
                    m_keyButtonMods.fileDropCB(*m_octave, noteToFiles, sampleLabel, myNoteName, myKeyName, safeButton, isDir, audioProcessor, noteToSample, synthNote,
                        noteToFile, buttonPalette);

                    resized();
                    m_keyButtonMods.resized();
                    sampleRefresh(*safeButton);
                });

        firstCount++;
    }

    // this is a loop for current octave key buttons
    // IMPORTANT!
    // i have a bunch of stuff in this loop that only needs to be initialized once. get it out of there
    int count = 0;
    int currentOctave = *m_octave;
    for (auto key : Constants::KEY_ORDER)
    {

        if (keyToNote.find(key) != keyToNote.end()) count++;
        auto it = keyToNote.find(key);
        if (it != keyToNote.end())
        {
            //-------keyboard------//
            char myChar = it->first;
            juce::String myKeyName = juce::String::charToString(myChar);
            juce::String myNoteName = it->second;

            // note-name
            auto* noteLabel = new NoteLabel(myNoteName);

            int buttonWidth = 60;
            int buttonHeight = 120;
            int labelHeight = 20;
            int spacing = 5;
            keyButtonX += buttonWidth + spacing + 35;

            // sample-name
            auto* sampleLabel = new SampleLabel("", myNoteName);

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
            grainDensitySlider.setLookAndFeel(&customLook);
            grainAmountSlider.setLookAndFeel(&customLook);
            grainLengthSlider.setLookAndFeel(&diySlider);
            grainAreaSlider.setLookAndFeel(&diySlider);


            addAndMakeVisible(buttonPalette);
            addAndMakeVisible(componentButton);
            addAndMakeVisible(buttonPalette.serialScheduleButton);
            addAndMakeVisible(octaveIncrement);
            addAndMakeVisible(octaveDecrement);
            addAndMakeVisible(grainDensityLabel);
            addAndMakeVisible(grainDensitySlider);
            addAndMakeVisible(grainAmountLabel);
            addAndMakeVisible(grainAmountSlider);
            addAndMakeVisible(grainLengthLabel);
            addAndMakeVisible(grainLengthSlider);
            addAndMakeVisible(grainAreaLabel);
            addAndMakeVisible(grainAreaSlider);
            addAndMakeVisible(chunkCrossfadeLabel);
            addAndMakeVisible(chunkCrossfadeSlider);
            addAndMakeVisible(noteLabel);
            addAndMakeVisible(*m_keyButton);
            addAndMakeVisible(sampleLabel);
            addAndMakeVisible(buttonPalette.synthToggleButton);
            addAndMakeVisible(buttonPalette.hanningToggleButton);
            addAndMakeVisible(buttonPalette.globalGainSlider);
            addAndMakeVisible(buttonPalette.globalGainSliderLabel);
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
            addAndMakeVisible(m_scrollableList);



            m_keyButtonMods.onWaveformButtonAdded = [this]() {
                resized();
                };

            componentButton.onClick = [this]() {
                    // (basically means 'if set'...)
                    if (onComponentButtonClicked)
                        onComponentButtonClicked();
                };

            octaveUp(octaveIncrement);
            octaveDown(octaveDecrement);

            serialScheduleAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "serialSchedule", buttonPalette.serialScheduleButton);
            grainDensityAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "grainDensity", grainDensitySlider);
            grainAmountAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "grainAmount", grainAmountSlider);
            chunkCrossfadeAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "chunkCrossfade", chunkCrossfadeSlider);

            hanningToggleAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "hanningToggle", *hanningToggleButton);
            compressorThresholdAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "globalGain",
                buttonPalette.globalGainSlider);

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
            globalGainSliderHandler(buttonPalette.globalGainSlider);

            noteLabels.add(noteLabel);


            sampleLabels.add(sampleLabel);

            audioProcessor.onThresholdChanged = [this](float v) {
                m_compressorWaveformComponent->updateCompressorThreshold(v);
                };
            audioProcessor.onRatioChanged = [this](float v) {
                m_compressorWaveformComponent->updateCompressorRatio(v);
                };

                    }
    }
    resized();

}

GranularInfinite::~GranularInfinite()
{
    grainDensitySlider.setLookAndFeel(nullptr);
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
    char char_key = static_cast<char>(key.getTextCharacter());
    auto it = keyToNote.find(char_key);
    if (it != keyToNote.end())
    {
        const juce::String& noteName = it->second;
        m_isCompressorTimer = true;
        compressorWaveformHandle();
        m_isPlayhead = true;
        //auto it2 = currentlyPressedKeys.find(char_key);
        //size_t index = Constants::KEY_ORDER.find(char_key);

        currentlyPressedKeys.insert(char_key);

        auto* button = m_keyButton->getKeyButtonByNote(noteName);

        button->setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
        button->repaint();
        if (auto* sampleName = noteToSample.getValue(noteName))
        {
            juce::String fuck = *sampleName;
            if (sampleName->isNotEmpty() && noteName.isNotEmpty())
            {
                audioProcessor.updateCurrentSamples(noteName, false);
                audioProcessor.startPlayback(noteName, fuck);
            }
            return true;
        }
        else {
            std::cout << "didn't find the noteName matching sample \n";
            return false;
        }
    }
    return false;
}

bool GranularInfinite::keyStateChanged(bool isKeyDown,
    Component* originatingComponent)
{

    if (!isKeyDown)
    {
        m_isCompressorTimer = false;

        for (auto it = currentlyPressedKeys.begin(); it != currentlyPressedKeys.end();)
        {
            char keyChar = *it;
            if (!juce::KeyPress::isKeyCurrentlyDown(keyChar))
            {
                juce::String noteName = keyToNote[keyChar];
                auto* button = m_keyButton->getKeyButtonByNote(noteName);
                if (keyChar == 'w' || keyChar == 'e' || keyChar == 't' || keyChar == 'y'
                    || keyChar == 'u' || keyChar == 'o' || keyChar == 'p')
                {
                    button->setColour(juce::TextButton::buttonColourId,
                        juce::Colours::black);
                    button->repaint();
                }
                else {
                    button->setColour(juce::TextButton::buttonColourId,
                        juce::Colours::white);
                    button->repaint();
                }
                it = currentlyPressedKeys.erase(it);

                auto sampleNameIt = keyToNote.find(keyChar);
                if (sampleNameIt != keyToNote.end()) {
                    if (auto* sampleName = noteToSample.getValue(sampleNameIt->second)) {
                        audioProcessor.updateCurrentSamples(noteName, true);
                        audioProcessor.stopPlayback(noteName, *sampleName);
                    }
                }
            }
            else
            {
                ++it;
            }
        }
    }
    return false;
}

void GranularInfinite::compressorWaveformTimer() {

    if (m_compressorWaveformComponent != nullptr )
    {
        m_compressorWaveformComponent->repaint();
    }
}

void GranularInfinite::compressorWaveformHandle() {

    if (m_compressorWaveformComponent == nullptr) {
        m_compressorWaveformComponent =
            std::make_unique<CompressorWaveformComponent>(audioProcessor.outputBuffer);
        addAndMakeVisible(*m_compressorWaveformComponent);
        m_compressorWaveformComponent->setBounds(650, 650, 500, 200);
    }
    
    if (m_isCompressorTimer)
        startTimerHz(60);
    else
        stopTimer();
}

// *TO-DO*
// - finish the 'sampleRefresh' function (make it work almost like 'octaveUp'.

// - use this after dropping file/folder.
// - lazily fixes SampleLabel issue.
void GranularInfinite::sampleRefresh(KeyButton& button) {

    auto& keyToNote = CreateKeyToNote(*m_octave);

    for (size_t i = 0; i < Constants::KEY_ORDER.size(); i++)
    {
        auto it = keyToNote.find(Constants::KEY_ORDER[i]);
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
        *m_octave = std::clamp(*m_octave + 1, 0, 8);
        keyToNote = CreateKeyToNote(*m_octave);

        for (size_t i = 0; i < Constants::KEY_ORDER.size(); i++)
        {
            auto it = keyToNote.find(Constants::KEY_ORDER[i]);
            if (it != keyToNote.end())
            {
                const auto& noteName = it->second;
                auto* sample = noteToSample.getValue(noteName);
                if (!sample)
                {
                    sampleLabels[i]->setButtonText("");
                }
                else {
                    juce::String raw = *sample;
                    sampleLabels[i]->setButtonText(raw);
                }
                noteLabels[i]->setText(noteName, juce::dontSendNotification);

            }
        }
        resized();
        m_keyButtonMods.resized();
        };

}

void GranularInfinite::octaveDown(juce::TextButton& button)
{
    button.onClick = [this] {
        *m_octave = std::clamp(*m_octave - 1, 0, 8);
        keyToNote = CreateKeyToNote(*m_octave);
        for (size_t i = 0; i < Constants::KEY_ORDER.size(); i++)
        {
            auto it = keyToNote.find(Constants::KEY_ORDER[i]);
            if (it != keyToNote.end())
            {
                const auto& noteName = it->second;
                auto* sample = noteToSample.getValue(noteName);
                if (!sample) {
                    sampleLabels[i]->setButtonText("");
                }
                else {
                    juce::String raw = *sample;
                    sampleLabels[i]->setButtonText(raw);
                }
                noteLabels[i]->setText(noteName, juce::dontSendNotification);

                //m_octaveKeyButtons[i]->setNoteName(noteName);
            }
        }
        resized();
        m_keyButtonMods.resized();
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

void GranularInfinite::globalGainSliderHandler(juce::Slider& slider) {
    slider.onValueChange = [this, &slider]() {
        auto* gainParam = audioProcessor.apvts.getParameter("globalGain");
        if (auto* floatParam = dynamic_cast<juce::RangedAudioParameter*>(gainParam)) {
            float normalized = floatParam->convertTo0to1((float)slider.getValue());
            gainParam->setValueNotifyingHost(normalized);
        }
    };
}

void GranularInfinite::timerCallback()
{
    if (m_compressorWaveformComponent != nullptr && m_isCompressorTimer)
    {
        m_compressorWaveformComponent->repaint();
    }

    // temoporarily disabled. create flag for this vvv
    if (m_isPlayhead)
        playheadPositionHandler();


}

void GranularInfinite::playheadPositionHandler()
{
    startTimerHz(60);
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

// | i dont think we are doing anything with this one
void GranularInfinite::synthToggleHandler(juce::TextButton& button)
{
    button.onClick = [this, &button] {
        bool isToggled = button.getToggleState();

        if (isToggled)
        {
            audioProcessor.m_grainAll = true;
            button.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
        }
        else {
            audioProcessor.m_grainAll = false;
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

    //m_keyButton->triggerResized(m_octave);
    //m_keyButton->setBounds(0, 100, 4000, 300);

    m_keyButtonMods.setBounds(0, 100, 4000, 300);
    m_scrollableList.setBounds(1600, 600, 4000, 300);

    buttonPalette.componentButton.setBounds(1600, y + 400, 80, 80);
    componentButton.setBounds(1600, y + 400, 80, 80);

    buttonPalette.sampleCompressorLabel.setBounds(1400, y + 300, 80, 100);
    buttonPalette.compressor.thresholdStruct.thresholdSlider.setBounds(1400, y + 400, 80, 100);
    buttonPalette.compressor.thresholdStruct.inc.setBounds(1475, y + 440, 20, 20);
    buttonPalette.compressor.thresholdStruct.dec.setBounds(1475, y + 460, 20, 20);
    buttonPalette.compressor.ratioSlider.setBounds(1400, y + 500, 80, 100);
    buttonPalette.compressor.attackCoeffSlider.setBounds(1400, y + 600, 80, 100);
    buttonPalette.compressor.releaseCoeffSlider.setBounds(1400, y + 700, 80, 100);
    buttonPalette.compressor.gainSlider.setBounds(1400, y + 800, 80, 100);
    buttonPalette.compressor.thresholdStruct.thresholdSliderLabel.setBounds(1300, y + 400, 80, 100);
    buttonPalette.compressor.ratioSliderLabel.setBounds(1300, y + 500, 80, 100);
    buttonPalette.compressor.attackCoeffSliderLabel.setBounds(1300, y + 600, 80, 100);
    buttonPalette.compressor.releaseCoeffSliderLabel.setBounds(1300, y + 700, 80, 100);
    buttonPalette.compressor.gainSliderLabel.setBounds(1300, y + 800, 80, 100);

    buttonPalette.serialScheduleButton.setBounds(x - 275, y + 450, buttonWidth, buttonHeight);
    buttonPalette.decrementButton.setBounds(x - 200, y + 350, buttonWidth, buttonHeight);
    buttonPalette.incrementButton.setBounds(x - 100, y + 350, buttonWidth, buttonHeight);

    buttonPalette.synthToggleButton.setBounds(x, y + 350, buttonWidth, buttonHeight);

    buttonPalette.hanningToggleButton.setBounds(x + 100, y + 350, buttonWidth, buttonHeight);

    buttonPalette.globalGainSliderLabel.setBounds(x + 210, y + 350, buttonWidth, labelHeight);
    buttonPalette.globalGainSlider.setBounds(x + 200, y + 400, 80, 100);

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


    juce::FlexItem f_grainDensityLabel(grainDensityLabel);
    inner1.items.add(
        f_grainDensityLabel
        .withHeight(30.0f)
        .withWidth(controlBounds.getWidth())
    );

    juce::FlexItem f_grainDensitySlider(grainDensitySlider);
    inner1.items.add(
        f_grainDensitySlider
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

    juce::FlexItem f_chunkCrossfadeLabel(chunkCrossfadeLabel);
    inner1.items.add(
        f_chunkCrossfadeLabel
        .withHeight(30.0f)
        .withWidth(controlBounds.getWidth())
    );

    juce::FlexItem f_chunkCrossfadeSlider(chunkCrossfadeSlider);
    inner1.items.add(
        f_chunkCrossfadeSlider
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
    m_keyButtonMods.drawUpwardCompressors(noteRange, *m_octave, keyControlX, y, buttonWidth, buttonHeight, spacing, buttonPalette);


    int keyButtonX = 75;
    int octaveStart = *m_octave * 12;

    if (m_keyButton == nullptr) {
        return;
    }

    m_keyButton->setBounds(0, 75, 4000, 135);
    m_keyButton->triggerResized(*m_octave);

    int count = 0;
    for (int i = octaveStart; i < octaveStart + Constants::DISPLAYED_NOTES_SIZE; ++i)
    {
        noteLabels[count]->setBounds(keyButtonX, y - 50, buttonWidth, labelHeight);

        sampleLabels[count]->setBounds(keyButtonX, y + buttonHeight, buttonWidth, labelHeight);

        count++;
        keyButtonX += buttonWidth + spacing + 35;   // move to next key
    }

}
 
