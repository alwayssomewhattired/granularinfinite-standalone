
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

class ButtonPalette : public juce::Component
{
public:
	ButtonPalette() : waveformState()
	{
		componentButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
		componentButton.setButtonText("switch");
		componentButton.setClickingTogglesState(true);

		incrementButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
		incrementButton.setButtonText("+");

		decrementButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
		decrementButton.setButtonText("-");

		synthToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
		synthToggleButton.setButtonText("granular");
		synthToggleButton.setClickingTogglesState(true);

		hanningToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
		hanningToggleButton.setButtonText("hanning");
		hanningToggleButton.setClickingTogglesState(true);

		grainSpacingLabel.setText("grain spacing", juce::dontSendNotification);
		grainSpacingLabel.setFont(juce::Font(16.0f, juce::Font::bold));
		grainSpacingLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
		grainSpacingLabel.setJustificationType(juce::Justification::centred);

		grainSpacingSlider.setSliderStyle(juce::Slider::LinearHorizontal);
		grainSpacingSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 80, 20);
		grainSpacingSlider.setRange(0.1, 48000.0, 0.1);
		grainSpacingSlider.setValue(1.0);

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




		// make a button for spotify events vvv
		spotifyButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
		spotifyButton.setButtonText("spotify");

		sourceDownloadButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
		sourceDownloadButton.setButtonText("download");

	}

	std::function<void()> onWaveformButtonAdded;

	void addWaveformButton(const juce::String& fileName, const juce::String& keyName, const juce::String& noteName, 
		std::function<void(juce::TextButton&)> onClick)
	{

		auto waveformButton = std::make_unique<WaveformButton>();
		waveformButton->fileName = fileName;
		waveformButton->keyName = keyName;
		waveformButton->noteName = noteName;

		waveformButton->waveformButton = std::make_unique<juce::TextButton>();
		auto* btnptr = waveformButton->waveformButton.get();
		waveformButton->waveformButton->setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
		waveformButton->waveformButton->setButtonText("waveform");
		waveformButton->waveformButton->setClickingTogglesState(true);

		waveformButton->waveformButton->onClick = [onClick, btnptr]() {
			onClick(*btnptr);
			};
		addAndMakeVisible(*waveformButton->waveformButton);

		waveformButtons[fileName] = std::move(waveformButton);
		if (onWaveformButtonAdded) {
			onWaveformButtonAdded();
		}
	}

	std::pair<std::unique_ptr<juce::Slider>, std::unique_ptr<juce::Label>> createFrequencyUpwardCompressor(double freq) {
		auto slider = std::make_unique<juce::Slider>();
		slider->setSliderStyle(juce::Slider::LinearVertical);
		slider->setTextBoxStyle(juce::Slider::TextBoxAbove, true, 80, 20);
		slider->setRange(0.0, 1.0, 0.01);
		slider->setValue(0.0);

		auto label = std::make_unique<juce::Label>();
		label->setText("freq expander", juce::dontSendNotification);
		label->setFont(juce::Font(16.0f, juce::Font::bold));
		label->setColour(juce::Label::textColourId, juce::Colours::green);
		label->setJustificationType(juce::Justification::centred);

		return std::make_pair(std::move(slider), std::move(label));
	}


	// file-name of current waveform button pressed. (warning: not a reference. be careful)
	juce::String waveformState;

	juce::TextButton componentButton;

	juce::TextButton incrementButton;
	juce::TextButton decrementButton;
	juce::TextButton synthToggleButton;
	juce::TextButton waveformButton;
	juce::TextButton spotifyButton;
	juce::TextButton sourceDownloadButton;
	juce::TextButton hanningToggleButton;

	struct WaveformButton {
		juce::String keyName;
		juce::String noteName;
		juce::String fileName;
		std::unique_ptr<juce::TextButton> waveformButton;
	};

	// file-name to waveformButton
	std::map<juce::String, std::unique_ptr<WaveformButton>> waveformButtons;

	juce::Slider grainSpacingSlider;
	juce::Slider grainAmountSlider;
	//juce::Slider frequencyUpwardCompressorSlider;

	juce::Label grainSpacingLabel;
	juce::Label grainAmountLabel;
	juce::Label grainLengthLabel;
	juce::Label grainPositionLabel;
	//juce::Label frequencyUpwardCompressorLabel;
};