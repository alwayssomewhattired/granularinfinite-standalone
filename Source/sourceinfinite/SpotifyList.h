
#pragma once
#include <JuceHeader.h>

struct SpotifyItem
{
	juce::String id;
	juce::String name;
};

class SpotifyList : public juce::Component,
	public juce::ListBoxModel
{
public:
	SpotifyList(const juce::String& title) : m_title(title)
	{
		SpotifyItem item{ "null", m_title };
		addAndMakeVisible(listBox);
		listBox.setModel(this);
		items.insert(0, item);
	}

	std::function<void(const SpotifyItem&)> onItemSelected;

	void updateItems(const juce::Array<SpotifyItem>& newItems)
	{
		items = newItems;
		listBox.updateContent();
		repaint();
	}

	void clearItems()
	{
		items.clear();
		listBox.updateContent();
	}

	int getNumRows() override { return items.size(); }

	void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override
	{
		if (selected) g.fillAll(juce::Colours::black);
		g.setColour(juce::Colours::green);
		g.drawText(items[row].name, 10, 0, width - 10, height, juce::Justification::centredLeft);
	}

	void listBoxItemClicked(int row, const juce::MouseEvent&) override
	{
		if (juce::isPositiveAndBelow(row, items.size()) && onItemSelected)
			onItemSelected(items[row]);
	}

	void resized() override
	{
		listBox.setBounds(getLocalBounds());
	}

private:
	juce::ListBox listBox;
	juce::Array<SpotifyItem> items;
	const juce::String& m_title;
};

