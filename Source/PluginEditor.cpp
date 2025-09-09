#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "KeyButton.h"
#include "OpenConsole.h"
#include "SampleLabel.h"
#include "NoteLabel.h"
#include "ButtonPalette.h"

//==============================================================================
GranularinfiniteAudioProcessorEditor::GranularinfiniteAudioProcessorEditor 
(GranularinfiniteAudioProcessor& p)
    : juce::AudioProcessorEditor(&p),
    audioProcessor(p)
{
    openConsole();
    keyToNote = CreateKeyToNote(octave); // add dynamic octave
    setWantsKeyboardFocus(true);
    addKeyListener(this);
    setSize(1000, 200);

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
        noteToSample[noteValue] = "";
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


            // octave-buttons
            juce::TextButton& octaveIncrement = buttonPalette.incrementButton;
            juce::TextButton& octaveDecrement = buttonPalette.decrementButton;

            // note-name
            auto* noteLabel = new NoteLabel(myNoteName);

            // piano-key
            auto* button = new KeyButton(myKeyName, myNoteName);
            button->setColour(juce::TextButton::buttonColourId, juce::Colours::white);
            button->setColour(juce::TextButton::buttonOnColourId, juce::Colours::lightgrey);
            button->setColour(juce::TextButton::textColourOffId, juce::Colours::black);

            // sample-name
            auto* label = new SampleLabel("");

            // myNoteName is not refreshing its values at octave change.
            button->setOnFileDropped([this, myNoteName, label](const juce::String& /*path*/, 
                const juce::String& name) {
                label->setText(name, juce::dontSendNotification);
                juce::String refinedNote = myNoteName.dropLastCharacters(1) + juce::String(octave);
                std::cout << refinedNote << "\n";
                noteToSample[refinedNote] = name;
                });

            addAndMakeVisible(octaveIncrement);
            addAndMakeVisible(octaveDecrement);
            addAndMakeVisible(noteLabel);
            addAndMakeVisible(button);
            addAndMakeVisible(label);

            octaveUp(octaveIncrement);
            octaveDown(octaveDecrement);

            noteLabels.add(noteLabel);
            keyButtons.add(button);
            sampleLabels.add(label);
        }
    }

}

GranularinfiniteAudioProcessorEditor::~GranularinfiniteAudioProcessorEditor()
{
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
        currentlyPressedKeys.insert(char_key);
        size_t index = order.find(char_key);
        if (index != std::string::npos)
        {
            auto* button = keyButtons[(int)index];

            button->setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
            button->repaint();
        }
        return true;
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
                it = currentlyPressedKeys.erase(it);
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
                auto sample_it = noteToSample.find(it->second);
                    noteLabels[i]->setText(it->second, juce::dontSendNotification);
                    if (sample_it != noteToSample.end())
                    {
                        sampleLabels[i]->setText(sample_it->second, juce::dontSendNotification);
                    }
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
                auto sample_it = noteToSample.find(it->second);
                noteLabels[i]->setText(it->second, juce::dontSendNotification);
                if (sample_it != noteToSample.end())
                {
                    sampleLabels[i]->setText(sample_it->second, juce::dontSendNotification);
                }
            }
        }
        };
}

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
            //keyButtons[i]->repaint();
        }
        else {
            keyButtons[i]->setBounds(x, y, buttonWidth, buttonHeight);
        }
            sampleLabels[i]->setBounds(x, y + buttonHeight, buttonWidth, labelHeight);

        x += buttonWidth + spacing;   // move to next key
    }
}