#pragma once
#include <JuceHeader.h>
#include "../BiMap.h"
#include "../Sample.h"

class ScrollableList : public juce::ListBoxModel
{
public:

	int getNumRows() override
	{
		return items.size();
	}

	void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool isRowSelected) override
	{
		if (isRowSelected)
			g.fillAll(juce::Colours::darkblue);

		g.setColour(juce::Colours::white);
		g.drawText(items[rowNumber], 10, 0, width, height, juce::Justification::centredLeft);
	}

	void listBoxItemClicked(int row, const juce::MouseEvent&) override
	{

		const juce::String& chosenFileName = items[row];

		//jassert(m_currentNoteToSample != nullptr);

		//if (auto* value = m_currentNoteToSample->getValue(m_key))
		//{
		//	juce::String blah = *value;
		//	std::cout << "value!!! " << blah << "\n";
		//}
		//else
		//{
		//	std::cout << "No value yet for key: " << m_key << "\n";
		//}

		for (auto& v : (m_samples)) {
			std::cout << "key 1: " << v->noteName << "\n";
			std::cout << "k 2: " << v->fileName << "\n";
		}

		std::cout << "m_key: " << m_key << "\n";
		std::cout << "chosenFileName: " << chosenFileName << "\n";
		//if (auto it = (*m_samples).find({ m_key, chosenFileName }) != m_samples->end()) {
		//	m_currentNoteToSample->set(m_key, items[row]);
		//	(*m_samples)[{ m_key, items[row] }]->isChosen = true;
		//}
		//else {
		//	std::cout << "couldn't find the correct sample \n";
		//}

		for (auto& v : m_samples) {
			if (v->noteName == m_key && v->fileName == chosenFileName) {
				m_currentNoteToSample->set(m_key, items[row]);
				v->isChosen = true;
			} else {
				v->isChosen = false;
			std::cout << "couldn't find the correct sample \n";
			}
		}

	}


	void setAudioConfig(BiMap<juce::String, juce::String>* currentNoteToSample, juce::String key, 
		std::shared_ptr<Sample> samples) {
		//std::shared_ptr<std::map<std::pair<juce::String, juce::String>, std::shared_ptr<Sample>>> samples) {
		m_samples.push_back(samples);
		m_currentNoteToSample = currentNoteToSample;
		m_key = key;
	}

	BiMap<juce::String, juce::String>* m_currentNoteToSample = nullptr;
	juce::String m_key;
	std::vector<juce::String> items;
	std::vector<std::shared_ptr<Sample>> m_samples;
	//std::unique_ptr<std::map<std::pair<juce::String, juce::String>, std::shared_ptr<Sample>>> m_samples;
};