#pragma once
#include "SearchableComboBox.h"
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "KeyToNote.h"

SearchableComboBox::SearchableComboBox()
{
	addAndMakeVisible(searchBox);
	searchBox.addListener(this);

	addAndMakeVisible(listBox);
	listBox.setModel(this);

	auto& notes = CreateNoteToMidi;

	for (const auto& [k, v] : notes)
	{
		allItems.add(k);
	}
	filteredItems = allItems;
}

void SearchableComboBox::resized()
{
	auto bounds = getLocalBounds();
	searchBox.setBounds(1500, 100, 40, 20);
	listBox.setBounds(1500, 120, 40, 200);
}

int SearchableComboBox::getNumRows() { return filteredItems.size(); }

void SearchableComboBox::paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected)
{
	if (selected)
		g.fillAll(juce::Colours::lightyellow);

	g.setColour(juce::Colours::green);
	g.drawText(filteredItems[row], 5, 0, w, h, juce::Justification::centredLeft);
}

void SearchableComboBox::listBoxItemClicked(int row, const juce::MouseEvent&)
{
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