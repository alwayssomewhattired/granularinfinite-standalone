
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class ButtonPalette : public juce::Component
{
public:
	ButtonPalette()
	{
		incrementButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
		incrementButton.setButtonText("+");

		decrementButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
		decrementButton.setButtonText("-");
	}


	juce::TextButton incrementButton;
	juce::TextButton decrementButton;

};