
#pragma once
#include <JuceHeader.h>
#include "ScrollableList.h"
#include "WaveformDisplay.h"

class ScrollableList;

class ScrollableListModel : public juce::ListBoxModel
{
public:

	explicit ScrollableListModel(ScrollableList& p) : parent(p) {}

	int getNumRows() override
	{
		return m_noteToFileNames[m_currentNote].size();
	}

	void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool isRowSelected) override
	{
		if (isRowSelected)
			g.fillAll(juce::Colours::darkblue);

		g.setColour(juce::Colours::white);
		std::cout << m_noteToFileNames[m_currentNote][rowNumber] << "\n";
		g.drawText(m_noteToFileNames[m_currentNote][rowNumber], 10, 0, width, height, juce::Justification::centredLeft);
	}

	void listBoxItemClicked(int row, const juce::MouseEvent&) override
	{
		
		const juce::String& chosenFileName = m_noteToFileNames[m_currentNote][row];

		std::cout << "m_currentNote: " << m_currentNote << "\n";
		std::cout << "chosenFileName: " << chosenFileName << "\n";

		for (auto& [k, vectorOfSamples] : m_samples) {
			if (k == m_currentNote) {
				for (auto& sample : vectorOfSamples) {
					if (sample->fileName == chosenFileName) {

						sample->isChosen = true;

						m_waveformDisplay->setBuffer(sample->fullBuffer);

					}
					else {

						sample->isChosen = false;

						std::cout << "couldn't find the correct sample \n";
					}
				}
			}

		}

		// get waveformdisplay object in here
		// pass audio buffer into waveformdisplay method.

	}


	void setAudioConfig(juce::String noteName, std::shared_ptr<Sample>& sample) {
		m_samples[noteName].push_back(std::move(sample));
		m_currentNote = noteName;
	}

	ScrollableList& parent;

	bool isSet = false;

	juce::String fuckyou = "fuckyou";

	juce::String m_currentNote;

	// note to filenames
	std::unordered_map<juce::String, std::vector<juce::String>> m_noteToFileNames;
	// notename to sample object(s)
	std::unordered_map<juce::String, std::vector<std::shared_ptr<Sample>>> m_samples;
	juce::Component::SafePointer<WaveformDisplay> m_waveformDisplay;
};