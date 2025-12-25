#pragma once
#include <JuceHeader.h>
#include "../BiMap.h"

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
		jassert(m_noteToSample != nullptr);

		if (auto* value = m_noteToSample->getValue(m_key))
		{
			juce::String blah = *value;
			std::cout << "value!!! " << blah << "\n";
		}
		else
		{
			std::cout << "No value yet for key: " << m_key << "\n";
		}

		m_noteToSample->set(m_key, items[row]);
	}


	void setAudioConfig(BiMap<juce::String, juce::String>* noteToSample, juce::String key) {
		m_noteToSample = noteToSample;
		m_key = key;
	}

	BiMap<juce::String, juce::String>* m_noteToSample = nullptr;
	juce::String m_key;
	std::vector<juce::String> items;
};