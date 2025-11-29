
#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <JuceHeader.h>
#include <juce_core/juce_core.h>
#include "ButtonPalette.h"



class KeyButtonMods : public juce::Component {

public:

    int drawUpwardCompressors(const std::vector<std::string>& noteRange, const int& octave,
  int& keyControlX, const int& y,
        const int& buttonWidth, const int& buttonHeight, const int& spacing, ButtonPalette& bp) {

        static std::unordered_map<std::string, juce::Slider*> prevSliders;

        for (int i = 0; i < noteRange.size() + 6; i++) {
            const std::string& note = noteRange[i % 12];
            std::string noteWithOctave;
            if (i >= 12)
                noteWithOctave = note + std::to_string(octave + 1);
            else
                noteWithOctave = note + std::to_string(octave);

            if (auto& it = m_frequencyUpwardCompressors.find(noteWithOctave); it != m_frequencyUpwardCompressors.end()) {
                juce::Slider& slider = *it->second.slider;
                if (auto& it = prevSliders.find(noteWithOctave); it != prevSliders.end()) {
                    slider.setVisible(false);
                    prevSliders.erase(noteWithOctave);
                }
                addAndMakeVisible(slider);
                slider.setBounds(keyControlX, y + 200, buttonWidth, 100);
                prevSliders[noteWithOctave] = &slider;
            }
            if (auto& it = m_frequencyUpwardCompressorLabels.find(noteWithOctave); it != m_frequencyUpwardCompressorLabels.end()) {
                addAndMakeVisible(*it->second);
                it->second->setBounds(keyControlX, y + 125, buttonWidth, buttonHeight - 25);
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

private:
    struct FrequencyUpwardCompressor {
        double frequency;
        std::unique_ptr<juce::Slider> slider;
    };

    // note to struct
    std::unordered_map<std::string, FrequencyUpwardCompressor> m_frequencyUpwardCompressors;
    // note to label
    std::unordered_map<std::string, std::unique_ptr<juce::Label>> m_frequencyUpwardCompressorLabels;
};