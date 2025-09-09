
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class SampleLabel : public juce::Label
{
public:
	SampleLabel(const juce::String fileName)
	{
		setJustificationType(juce::Justification::centred);
		setFont(juce::Font(12.0f));
		setColour(juce::Label::textColourId, juce::Colours::green);
		setText(fileName, juce::dontSendNotification);
	}

	~SampleLabel() override = default;
private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleLabel)
};
