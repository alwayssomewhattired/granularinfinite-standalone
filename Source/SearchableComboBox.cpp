#pragma once
#include "SearchableComboBox.h"
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "KeyToNote.h"

SearchableComboBox::SearchableComboBox(const bool realtimeResults)
{
	addAndMakeVisible(searchBox);
	searchBox.addListener(this);

	addAndMakeVisible(listBox);
	listBox.setModel(this);

	if (realtimeResults)
	{
		auto& notes = CreateNoteToMidi;

		for (const auto& [k, v] : notes)
		{
			allItems.add(k);
		}

		filteredItems = allItems;
	}

}

void SearchableComboBox::resized()
{
	if (realtimeResults)
	{
		searchBox.setBounds(0, 0, 200, 40);
		listBox.setBounds(0, 40, 200, 200);
	}
	else
	{
		searchBox.setBounds(0, 0, 50, 40);
		listBox.setBounds(0, 40, 50, 200);
	}
	searchBox.setFont(juce::Font(26.0f));
	searchBox.setColour(juce::TextEditor::textColourId, juce::Colours::green);
	searchBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
	searchBox.setJustification(juce::Justification::centredTop);
	
}

int SearchableComboBox::getNumRows() { return filteredItems.size(); }

std::vector<double> SearchableComboBox::getFrequencies()
{
	std::vector<double> chosenFreqs;
	std::map<std::string, double> myMap = createNoteToFreq();
	
	for (const int& n : toggledRows)
	{
		chosenFreqs.push_back(myMap[filteredItems[n].toStdString()]);
	}

	return chosenFreqs;
}

void SearchableComboBox::paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected)
{
	const bool isToggled = toggledRows.contains(row);

	if (isToggled)
	{
		g.fillAll(juce::Colours::yellow);
		g.setColour(juce::Colours::black);
	}
	else
	{
		g.setColour(juce::Colours::green);
	}

	g.drawText(filteredItems[row], 5, 0, w, h, juce::Justification::centredLeft);
}

void SearchableComboBox::listBoxItemClicked(int row, const juce::MouseEvent&)
{
	if (toggledRows.contains(row))
		toggledRows.removeAllInstancesOf(row);
	else
		toggledRows.add(row);
	listBox.repaintRow(row);

	if (onItemSelected)
		onItemSelected(filteredItems[row]);
}

void SearchableComboBox::textEditorTextChanged(juce::TextEditor& editor)
{
	const auto query = editor.getText();
	filteredItems.clear();

	for (const auto& item : allItems)
	{
		if (item.containsIgnoreCase(query))
			filteredItems.add(item);
	}

	listBox.updateContent();
}