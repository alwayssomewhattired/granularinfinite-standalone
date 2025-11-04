
#pragma once
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>

class SearchableComboBox : public juce::Component, private juce::TextEditor::Listener, private juce::ListBoxModel
{
public:
	SearchableComboBox();

	void resized() override;

	int getNumRows() override;

	std::vector<double> getFrequencies();

	void paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected) override;

	void listBoxItemClicked(int row, const juce::MouseEvent&) override;

	void textEditorTextChanged(juce::TextEditor& editor) override;

	std::function<void(juce::String)> onItemSelected;

private:
	juce::TextEditor searchBox;
	juce::ListBox listBox;
	juce::StringArray allItems;
	juce::StringArray filteredItems;

	// stores indices of rows that are selected
	juce::Array<int> toggledRows;
};