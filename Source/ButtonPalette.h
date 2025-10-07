
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
//#include "PluginProcessor.h"

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

		grainSpacingLabel.setText("grain spacing", juce::dontSendNotification);
		grainSpacingLabel.setFont(juce::Font(16.0f, juce::Font::bold));
		grainSpacingLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
		grainSpacingLabel.setJustificationType(juce::Justification::centred);

		grainSpacingSlider.setSliderStyle(juce::Slider::LinearHorizontal);
		grainSpacingSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 80, 20);
		grainSpacingSlider.setRange(1.0, 48000.0, 1.0);
		grainSpacingSlider.setValue(20.0);
		
		grainAmountLabel.setText("grain amount", juce::dontSendNotification);
		grainAmountLabel.setFont(juce::Font(16.0f, juce::Font::bold));
		grainAmountLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
		grainAmountLabel.setJustificationType(juce::Justification::centred);

		grainAmountSlider.setSliderStyle(juce::Slider::LinearHorizontal);
		grainAmountSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 80, 20);
		grainAmountSlider.setRange(1.0, 256.0, 1.0);
		grainAmountSlider.setValue(1.0);

		grainPositionLabel.setText("grain area", juce::dontSendNotification);
		grainPositionLabel.setFont(juce::Font(16.0f, juce::Font::bold));
		grainPositionLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
		grainPositionLabel.setJustificationType(juce::Justification::centred);

		grainLengthLabel.setText("grain length", juce::dontSendNotification);
		grainLengthLabel.setFont(juce::Font(16.0f, juce::Font::bold));
		grainLengthLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
		grainLengthLabel.setJustificationType(juce::Justification::centred);
	}


	juce::TextButton incrementButton;
	juce::TextButton decrementButton;
	juce::TextButton synthToggleButton;

	juce::Slider grainSpacingSlider;
	juce::Slider grainAmountSlider;

	juce::Label grainSpacingLabel;
	juce::Label grainAmountLabel;
	juce::Label grainLengthLabel;
	juce::Label grainPositionLabel;
};