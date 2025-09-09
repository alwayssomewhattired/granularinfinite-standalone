
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class NoteLabel : public juce::Label
{
public:
	NoteLabel(const juce::String myNote)
	{
		setJustificationType(juce::Justification::centred);
		setFont(juce::Font(12.0f));
		setColour(juce::Label::textColourId, juce::Colours::green);
		setText(myNote, juce::dontSendNotification);
	}

	~NoteLabel() override = default;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteLabel)
};