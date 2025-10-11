
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

		waveformButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
		waveformButton.setButtonText("waveform");
		waveformButton.setClickingTogglesState(true);
		waveformButton.onClick = [this]() {
			if (onToggleWaveform) onToggleWaveform(waveformButton.getToggleState());
			};

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

		std::function<void(bool)> onToggleWaveform;
		std::function<void()> onWaveformButtonAdded;

	void addWaveformButton(const juce::String& fileName, std::function<void()> onClick)
	{
		auto button = std::make_unique<juce::TextButton>(fileName);
		button->setColour(juce::TextButton::buttonColourId, juce::Colours::green);
		button->setButtonText("waveform");
		button->setClickingTogglesState(true);
		button->onClick = std::move(onClick);
		addAndMakeVisible(*button);
		waveformButtons[fileName] = std::move(button);
		if (onWaveformButtonAdded) {
			onWaveformButtonAdded();
		}
		std::cout << "do you knnow? \n";
	}

	//void ButtonPalette::resized()
	//{
	//	int x = 300;
	//	int y = 100;
	//	int buttonWidth = 60;
	//	int buttonHeight = 120;
	//	int labelHeight = 20;

	//	auto area = getLocalBounds();
	//	for (auto& pair : waveformButtons)
	//	{
	//		std::cout << "within resized \n";
	//		pair.second->setBounds(x + 100, y + 400, buttonWidth, labelHeight);
	//	}
	//}



	juce::TextButton incrementButton;
	juce::TextButton decrementButton;
	juce::TextButton synthToggleButton;
	juce::TextButton waveformButton;
	// file-name to waveformButton
	std::map<juce::String, std::unique_ptr<juce::TextButton>> waveformButtons;

	juce::Slider grainSpacingSlider;
	juce::Slider grainAmountSlider;

	juce::Label grainSpacingLabel;
	juce::Label grainAmountLabel;
	juce::Label grainLengthLabel;
	juce::Label grainPositionLabel;
};