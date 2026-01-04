
#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <JuceHeader.h>
#include <juce_core/juce_core.h>
#include "ButtonPalette.h"
#include "ScrollableList.h"
#include "../PluginProcessor.h"
#include "../Sample.h"



class KeyButtonMods : public juce::Component {

public:

    int drawUpwardCompressors(const std::vector<std::string>& noteRange, const int& octave,
  int& keyControlX, const int& y,
        const int& buttonWidth, const int& buttonHeight, const int& spacing, ButtonPalette& bp) {

        static std::unordered_map<std::string, juce::Slider*> prevSliders;

        for (auto [k, v] : prevSliders) {
            v->setVisible(false);
        }

        for (int i = 0; i < noteRange.size() + 6; i++) {
            const std::string& note = noteRange[i % 12];
            std::string noteWithOctave;
            if (i >= 12)
                noteWithOctave = note + std::to_string(octave + 1);
            else
                noteWithOctave = note + std::to_string(octave);

            if (auto it = m_frequencyUpwardCompressors.find(noteWithOctave); it != m_frequencyUpwardCompressors.end()) {
                juce::Slider& slider = *it->second.slider;
                    addAndMakeVisible(slider);
                    slider.setBounds(keyControlX, 200, buttonWidth, 100);
                    prevSliders[noteWithOctave] = &slider;
            }
            if (auto it = m_frequencyUpwardCompressorLabels.find(noteWithOctave); it != m_frequencyUpwardCompressorLabels.end()) {
                addAndMakeVisible(*it->second);
                it->second->setBounds(keyControlX, 125, buttonWidth, buttonHeight - 25);
            }

            keyControlX += buttonWidth + spacing + 35;
        }
        return keyControlX;
    }

    void addFrequencyUpwardCompressor(const std::string& note, const double& freq, std::unique_ptr<juce::Slider> slider) {
        m_frequencyUpwardCompressors[note] = FrequencyUpwardCompressor{ freq, std::move(slider) };
    }

    void addFrequencyUpwardCompressorLabel(const std::string& note, std::unique_ptr<juce::Label> label) {
        m_frequencyUpwardCompressorLabels[note] = std::move(label);
    }

    void cleanUp() {
        for (auto& [k, v] : m_frequencyUpwardCompressors) {
            v.slider->setLookAndFeel(nullptr);
        }
    }

    void fileDropCB(int& octave, std::map<juce::String, juce::Array<juce::File>>& noteToFiles, SampleLabel* sampleLabel, const juce::String& myNoteName,
        const juce::String& myKeyName, KeyButton& button, const bool& isDir, GranularinfiniteAudioProcessor& audioProcessor, BiMap<juce::String, 
        juce::String>& noteToSample, juce::String& synthNote, std::map<juce::String, std::unique_ptr<juce::File>>& noteToFile, ButtonPalette& buttonPalette, 
        WaveformDisplay& waveformDisplay, ScrollableList& m_scrollableList) {

        const std::map<char, juce::String>& keyToNote = CreateKeyToNote(octave);

        juce::String fullPath;
        juce::String name;
        juce::String refinedNote;

        for (const auto& [k, v] : noteToFiles) {

            // filenames for scrollable list
            std::vector<juce::String> fileNames;
            for (const juce::File& file : v) {
                fileNames.push_back(file.getFileNameWithoutExtension());
            }
            
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

            for (const juce::File& audioFile : v) {
                fullPath = audioFile.getFullPathName();
                name = audioFile.getFileNameWithoutExtension();
                //button.setTrimmedFileName(name);
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

                // only handle buttons for file-selection-toggle in keyButtonMods

                m_scrollableList.setScrollableList(audioProcessor, refinedNote, file, noteToSample, fileNames);

                buttonPalette.addWaveformButton(refinedName, myKeyName, refinedNote, [](juce::TextButton& waveformButton) {
                    waveformButton.setToggleState(true, juce::dontSendNotification);
                    std::cout << "clicked!\n";
                    });

    
                // temporarily disblef
                //addWaveformButtonCB(refinedName, refinedNote, myKeyName, file, button, buttonPalette, waveformDisplay, audioProcessor);

            }
        }
    }


private:
    struct FrequencyUpwardCompressor {
        double frequency;
        std::unique_ptr<juce::Slider> slider;
    };


    //  - * button * - the KeyButton
    void addWaveformButtonCB(juce::String& refinedName, juce::String& refinedNote, const juce::String& refinedKey, juce::File file,
        KeyButton& button, ButtonPalette& buttonPalette, WaveformDisplay& waveformDisplay, GranularinfiniteAudioProcessor& audioProcessor) {

        juce::String& state = buttonPalette.waveformState;

        if (button.getToggleState())
        {
            std::cout << "i doubt it\n";
            if (state.isNotEmpty())
            {
                if (auto it = buttonPalette.waveformButtons.find(state); it != buttonPalette.waveformButtons.end())
                {
                    it->second->waveformButton->setToggleState(false, juce::dontSendNotification);
                }
            }
            state = refinedName;
            waveformDisplay.setBuffer(audioProcessor.getSampleBuffer(refinedNote, refinedName));

            std::shared_ptr<Sample> samplePtr = nullptr;
            for (auto& v : audioProcessor.samples) {
                if (v.second->fileName == file.getFileNameWithoutExtension())
                    samplePtr = v.second;
            }

            waveformDisplay.setSample(samplePtr);

            waveformDisplay.setPlayheadPosition();
            button.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
        }
        else {
            std::cout << "no toggle state :( \n";
            state = juce::String();
            waveformDisplay.clear();
        }
    }



    // note to struct
    std::unordered_map<std::string, FrequencyUpwardCompressor> m_frequencyUpwardCompressors;
    // note to label
    std::unordered_map<std::string, std::unique_ptr<juce::Label>> m_frequencyUpwardCompressorLabels;
};