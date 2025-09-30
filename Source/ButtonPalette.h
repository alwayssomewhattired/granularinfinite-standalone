
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

		synthToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
		synthToggleButton.setButtonText("mode");
		synthToggleButton.setClickingTogglesState(true);

		grainSpacingSlider.setSliderStyle(juce::Slider::LinearHorizontal);
		grainSpacingSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 80, 20);
		grainSpacingSlider.setRange(1.0, 48000.0, 1.0);
		grainSpacingSlider.setValue(20.0);
		
		grainAmountSlider.setSliderStyle(juce::Slider::LinearHorizontal);
		grainAmountSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 80, 20);
		grainAmountSlider.setRange(1.0, 256.0, 1.0);
		grainAmountSlider.setValue(1.0);
	}


	juce::TextButton incrementButton;
	juce::TextButton decrementButton;
	juce::TextButton synthToggleButton;

	juce::Slider grainSpacingSlider;
	juce::Slider grainAmountSlider;
};