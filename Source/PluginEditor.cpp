#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "KeyButton.h"
#include "OpenConsole.h"
#include "SampleLabel.h"
#include "NoteLabel.h"
#include "ButtonPalette.h"
#include "DualThumbSlider.h"

//==============================================================================
GranularinfiniteAudioProcessorEditor::GranularinfiniteAudioProcessorEditor 
(GranularinfiniteAudioProcessor& p)
    : juce::AudioProcessorEditor(&p),
    audioProcessor(p),
    grainLengthSlider(std::tuple<double, double, double>(128.0, 48000.0, 1.0)),
    grainSpacingControl(buttonPalette.grainSpacingSlider),
    grainAmountControl(buttonPalette.grainAmountSlider),
    grainPositionControl(buttonPalette.grainPositionSlider)
{

    keyToNote = CreateKeyToNote(octave); // add dynamic octave
    setWantsKeyboardFocus(true);
    addKeyListener(this);
    setSize(1000, 200);

    grainLengthSliderHandler();

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
    // i think the problem is this is a string. it should be a juce::string!
    for (std::string noteValue : notes)
    {
        noteToSample.set(noteValue, "");
    }
    const std::string order = "awsedftgyhujkolp;'";
    int count = 0;
    for (auto key : order)
    {
        if (keyToNote.find(key) != keyToNote.end()) count++;
        auto it = keyToNote.find(key);
        if (it != keyToNote.end())
        {
            char myChar = it->first;
            juce::String myKeyName = juce::String::charToString(myChar);
            juce::String myNoteName = it->second;


            // control-buttons
            juce::TextButton& octaveIncrement = buttonPalette.incrementButton;
            juce::TextButton& octaveDecrement = buttonPalette.decrementButton;

            grainSpacingControl.setLookAndFeel(&customLook);
            grainAmountControl.setLookAndFeel(&customLook);
            grainLengthSlider.setLookAndFeel(&diySlider);
            grainPositionControl.setLookAndFeel(&customLook);

            // note-name
            auto* noteLabel = new NoteLabel(myNoteName);

            // piano-key
            auto* button = new KeyButton(myKeyName, myNoteName);
            button->setColour(juce::TextButton::buttonColourId, juce::Colours::white);
            button->setColour(juce::TextButton::buttonOnColourId, juce::Colours::lightgrey);
            button->setColour(juce::TextButton::textColourOffId, juce::Colours::black);

            // sample-name
            auto* label = new SampleLabel("");
            button->setOnFileDropped([this, myNoteName, label](const juce::String& fullPath, 
                const juce::String& name) {
                juce::String refinedNote = myNoteName.dropLastCharacters(1) + juce::String(octave);
                label->setButtonText(name);
                noteToSample.set(refinedNote, name);
                juce::File file(fullPath);
                juce::String refinedName = file.getFileNameWithoutExtension();
                label->file = refinedName;
                synthNote = refinedNote;
                noteToFile[refinedNote] = std::make_unique<juce::File>(fullPath);
                audioProcessor.loadFile(file, refinedNote, "false");
                });

            addAndMakeVisible(octaveIncrement);
            addAndMakeVisible(octaveDecrement);
            addAndMakeVisible(grainSpacingControl);
            addAndMakeVisible(grainAmountControl);
            addAndMakeVisible(grainLengthSlider);
            addAndMakeVisible(grainPositionControl);
            addAndMakeVisible(noteLabel);
            addAndMakeVisible(button);
            addAndMakeVisible(label);
            addAndMakeVisible(buttonPalette.synthToggleButton);

            octaveUp(octaveIncrement);
            octaveDown(octaveDecrement);
            grainSpacingAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "grainSpacing", grainSpacingControl);
            grainAmountAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "grainAmount", grainAmountControl);
            grainPositionAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "grainPosition", grainPositionControl);
           
            synthToggleHandler(buttonPalette.synthToggleButton);
            sampleLabelHandler(*label);


            noteLabels.add(noteLabel);
            keyButtons.add(button);
            sampleLabels.add(label);
        }
    }

}

GranularinfiniteAudioProcessorEditor::~GranularinfiniteAudioProcessorEditor()
{
    grainSpacingControl.setLookAndFeel(nullptr);
    grainAmountControl.setLookAndFeel(nullptr);
    grainPositionControl.setLookAndFeel(nullptr);
}

//==============================================================================

bool GranularinfiniteAudioProcessorEditor::keyPressed(const juce::KeyPress& key,
    Component* originatingComponent)
{

    const std::string order = "awsedftgyhujkolp;'";
    char char_key = static_cast<char>(key.getTextCharacter());
    auto it = keyToNote.find(char_key);
    if (it != keyToNote.end())
    {
    auto it2 = currentlyPressedKeys.find(char_key);
    size_t index = order.find(char_key);
    if (index != std::string::npos)
    {
        auto* button = keyButtons[(int)index];
        if (audioProcessor.synthToggle)
        {
            if (it2 == currentlyPressedKeys.end())
            {
                const int midi_note = CreateNoteToMidi[it->second];
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
            if (auto* sampleName = noteToSample.getValue(it->second))
            {
                if (sampleName->isNotEmpty())

                {
                    audioProcessor.startPlayback(it->second);
                }
                return true;
            }
            else {
                std::cout << "sample failure\n";
                return false;
            }
        }
    }
    return false;
}

bool GranularinfiniteAudioProcessorEditor::keyStateChanged(bool isKeyDown,
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

void GranularinfiniteAudioProcessorEditor::octaveUp(juce::TextButton& button)
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
    };
}

void GranularinfiniteAudioProcessorEditor::octaveDown(juce::TextButton& button)
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
        };
}

void GranularinfiniteAudioProcessorEditor::grainLengthSliderHandler()
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

void GranularinfiniteAudioProcessorEditor::sampleLabelHandler(SampleLabel& button)
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

void GranularinfiniteAudioProcessorEditor::synthToggleHandler(juce::TextButton& button)
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


void GranularinfiniteAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void GranularinfiniteAudioProcessorEditor::resized()
{
    int x = 300;                  // starting x position
    int y = 100;                  // starting y position
    int buttonWidth = 60;
    int buttonHeight = 120;
    int labelHeight = 20;
    int spacing = 5;

    buttonPalette.decrementButton.setBounds(x - 200, y + 400, buttonWidth, buttonHeight);
    buttonPalette.incrementButton.setBounds(x - 100, y + 400, buttonWidth, buttonHeight);
    buttonPalette.grainSpacingSlider.setBounds(x - 200, y + 600, buttonWidth + 100, buttonHeight - 50);
    buttonPalette.grainAmountSlider.setBounds(x - 200, 650, buttonWidth + 100, buttonHeight - 50);
    grainLengthSlider.setBounds(x - 200, y + 700, buttonWidth + 100, buttonHeight - 50);
    buttonPalette.synthToggleButton.setBounds(x, y + 400, buttonWidth, buttonHeight);

    for (int i = 0; i < keyButtons.size(); ++i)
    {
        noteLabels[i]->setBounds(x, y - 50, buttonWidth, labelHeight);
        if (i == 1 || i == 3 || i == 6 || i == 8 || i == 10 || i == 13 || i == 15)
        {
            keyButtons[i]->setBounds(x + 2, y - 10, buttonWidth - 5, buttonHeight - 25);
            keyButtons[i]->setColour(juce::TextButton::buttonColourId, juce::Colours::black);
            keyButtons[i]->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
            keyButtons[i]->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            keyButtons[i]->setColour(juce::ComboBox::outlineColourId, juce::Colours::white);
        }
        else {
            keyButtons[i]->setBounds(x, y, buttonWidth, buttonHeight);
        }
        sampleLabels[i]->setBounds(x, y + buttonHeight, buttonWidth, labelHeight);

        x += buttonWidth + spacing;   // move to next key
    }
}
