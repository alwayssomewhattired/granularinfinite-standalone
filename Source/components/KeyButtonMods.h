
#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <JuceHeader.h>
#include <juce_core/juce_core.h>
#include "ButtonPalette.h"
#include "ScrollableList.h"



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
        KeyButton* button, const bool& isDir, GranularinfiniteAudioProcessor& audioProcessor, BiMap<juce::String, juce::String>& noteToSample,
        juce::String& synthNote, std::map<juce::String, std::unique_ptr<juce::File>>& noteToFile, ButtonPalette& buttonPalette, WaveformDisplay& waveformDisplay) {

        const std::map<char, juce::String>& keyToNote = CreateKeyToNote(octave);

        juce::String fullPath;
        juce::String name;
        juce::String refinedNote;

        for (const auto& [k, v] : noteToFiles) {
            if (v.size() > 1) {
                // in here, make the button and 
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
                m_scrollableList.setAudioConfig(&noteToSample, refinedNote);
                m_scrollableList.items = { fileNames };
                m_listBox.setModel(&m_scrollableList);
                m_listBox.setRowHeight(20);
                addAndMakeVisible(m_listBox);
            }
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

                addWaveformButtonCB(refinedName, refinedNote, samplePtr, *button, buttonPalette, waveformDisplay, audioProcessor);
            }
        }
    }

    void resized() override
    {
        auto area = getLocalBounds();
        m_listBox.setBounds(area.removeFromLeft(area.getWidth() * 0.07f));
    }


private:
    struct FrequencyUpwardCompressor {
        double frequency;
        std::unique_ptr<juce::Slider> slider;
    };

    void addWaveformButtonCB(juce::String& refinedName, juce::String& refinedNote, GranularinfiniteAudioProcessor::Sample* samplePtr, 
        juce::TextButton& button, ButtonPalette& buttonPalette, WaveformDisplay& waveformDisplay, GranularinfiniteAudioProcessor& audioProcessor) {
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
            waveformDisplay.setBuffer(audioProcessor.getSampleBuffer(refinedNote));
            waveformDisplay.setSample(samplePtr);

            waveformDisplay.setPlayheadPosition();
            button.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
        }
        else {
            state = juce::String();
            waveformDisplay.clear();
        }
    }

    ScrollableList m_scrollableList;
    juce::ListBox m_listBox;



    // note to struct
    std::unordered_map<std::string, FrequencyUpwardCompressor> m_frequencyUpwardCompressors;
    // note to label
    std::unordered_map<std::string, std::unique_ptr<juce::Label>> m_frequencyUpwardCompressorLabels;
};