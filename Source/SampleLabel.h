
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class SampleLabel : public juce::TextButton
{
public:
	SampleLabel(const juce::String fileName, const juce::String note)
	{
		file = fileName;
		m_note = note;
		setButtonText(fileName);
		setColour(juce::TextButton::textColourOnId, juce::Colours::green);
		setColour(juce::TextButton::textColourOffId, juce::Colours::green);
		setClickingTogglesState(true);
	}

	void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
	{
		g.fillAll(juce::Colours::black);
		//if (isButtonDown || getToggleState())
		if (getToggleState())

			g.setColour(juce::Colours::lightgreen);
		else if (isMouseOver)
			g.setColour(juce::Colours::lightgreen);
		else
			g.setColour(juce::Colours::green);
		g.setFont(juce::Font(14.0f));
		g.drawText(getButtonText(), getLocalBounds(), juce::Justification::centred);
	};

	// I assume this is the file name
	juce::String file;

	~SampleLabel() override = default;
private:
	juce::String m_note;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleLabel)
};
