
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

struct CustomLookAndFeel : public juce::LookAndFeel_V4
{
	juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
	{
		auto layout = LookAndFeel_V4::getSliderLayout(slider);

		layout.textBoxBounds.setY(layout.textBoxBounds.getY() + 10);

		return layout;
	}
};

struct DiySlider : public juce::LookAndFeel_V4
{
	juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
	{
		auto layout = LookAndFeel_V4::getSliderLayout(slider);

		layout.textBoxBounds.setY(layout.textBoxBounds.getY() - 20);

		return layout;
	}
};