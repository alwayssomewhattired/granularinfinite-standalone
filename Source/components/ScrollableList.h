#pragma once
#include <JuceHeader.h>
#include "../BiMap.h"
#include "../Sample.h"


struct ScrollableListModel : public juce::ListBoxModel
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

		for (auto& v : (m_samples)) {
			std::cout << "key 1: " << v->noteName << "\n";
			std::cout << "k 2: " << v->fileName << "\n";
		}

		std::cout << "m_key: " << m_key << "\n";
		std::cout << "chosenFileName: " << chosenFileName << "\n";

		for (auto& v : m_samples) {
			if (v->noteName == m_key && v->fileName == chosenFileName) {
				m_currentNoteToSample->set(m_key, items[row]);
				v->isChosen = true;
			}
			else {
				v->isChosen = false;
				std::cout << "couldn't find the correct sample \n";
			}
		}

	}


	void setAudioConfig(BiMap<juce::String, juce::String>* currentNoteToSample, juce::String key,
		std::shared_ptr<Sample> samples) {
		m_samples.push_back(samples);
		m_currentNoteToSample = currentNoteToSample;
		m_key = key;
	}

	BiMap<juce::String, juce::String>* m_currentNoteToSample = nullptr;
	juce::String m_key;
	std::vector<juce::String> items;
	std::vector<std::shared_ptr<Sample>> m_samples;
};


class ScrollableList : public juce::Component
{
public:

	void setScrollableList(GranularinfiniteAudioProcessor& audioProcessor, const juce::String& refinedNote, const juce::File& file,
		BiMap<juce::String, juce::String>& noteToSample, std::vector<juce::String>& fileNames) {

		std::shared_ptr<Sample>& samplePtr = audioProcessor.loadFile(file, refinedNote, "false");
		std::vector<juce::String>& items = m_scrollableList.items;
		if (samplePtr == nullptr) std::cout << "this is null ptr\n";
		m_scrollableList.setAudioConfig(&noteToSample, refinedNote, std::move(samplePtr));
		for (auto k : fileNames) {
			std::cout << k << "\n";
			items.push_back(k);
		}

		// erase-remove idiom
		std::sort(items.begin(), items.end());
		items.erase(std::unique(items.begin(), items.end()), items.end());

		m_listBox.setModel(&m_scrollableList);
		m_listBox.setRowHeight(20);
		addAndMakeVisible(m_listBox);
	}

	void resized() override
	{
		auto area = getLocalBounds();
		m_listBox.setBounds(area.removeFromLeft(area.getWidth() * 0.07f));
	}

	ScrollableListModel m_scrollableList;
	juce::ListBox m_listBox;
};