
#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"


class GrainPositionControl : public juce::Slider
{
public:
	GrainPositionControl(GranularinfiniteAudioProcessor& p)
	{
		setSliderStyle(juce::Slider::LinearHorizontal);
		setTextBoxStyle(juce::Slider::TextBoxAbove, true, 80, 20);
		setRange(0.0, (double)p.getMaxFileSize(), 1.0);
		setValue(0.0);
	}

};