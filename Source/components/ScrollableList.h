#pragma once
#include <JuceHeader.h>
#include "../BiMap.h"
#include "../Sample.h"
#include "ScrollableListModel.h"


class ScrollableList : public juce::Component
{
public:

	ScrollableList() : m_scrollableListModel(*this) { }

	void setScrollableList(GranularinfiniteAudioProcessor& audioProcessor, const juce::String& refinedNote, const juce::File file,
		std::vector<juce::String> fileNames, WaveformDisplay& waveformDisplay) {

		m_scrollableListModel.m_waveformDisplay = &waveformDisplay;

		std::shared_ptr<Sample> samplePtr = audioProcessor.loadFile(file, refinedNote, "false");
		std::unordered_map<juce::String, std::vector<juce::String>>& items = m_scrollableListModel.m_noteToFileNames;
		m_scrollableListModel.setAudioConfig(refinedNote, std::move(samplePtr));
		m_scrollableListModel.m_currentNote = refinedNote;
		for (auto k : fileNames) {
			items[refinedNote].push_back(k);
		}

		// erase-remove idiom *temporarily removed
		//std::sort(items.begin(), items.end());
		//items.erase(std::unique(items.begin(), items.end()), items.end());

		if (m_scrollableListModel.isSet == false) {
			m_listBox.setModel(&m_scrollableListModel);
			m_listBox.setRowHeight(20);
			addAndMakeVisible(m_listBox);
			m_scrollableListModel.isSet = true;
		}
	}

	void resized() override
	{
		auto area = getLocalBounds();
		m_listBox.setBounds(area.removeFromLeft(area.getWidth() * 0.07f));
	}

	ScrollableListModel m_scrollableListModel;
	juce::ListBox m_listBox;
};