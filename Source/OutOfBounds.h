
#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginProcessor.h"

class OutOfBounds : public juce::Component
{
public:
	OutOfBounds(GranularinfiniteAudioProcessor& p) : audioProcessor(p) {

		m_compressorMixSliderLabel.setText("mix", juce::dontSendNotification);
		m_compressorMixSliderLabel.setFont(juce::Font(16.0f, juce::Font::bold));
		m_compressorMixSliderLabel.setColour(juce::Label::textColourId, juce::Colours::green);
		m_compressorMixSliderLabel.setJustificationType(juce::Justification::centred);

		m_compressorMixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
		m_compressorMixSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 80, 20);
		m_compressorMixSlider.setRange(0.0, 1.0, 0.01);
		m_compressorMixSlider.setValue(0.0);

		addAndMakeVisible(m_compressorMixSlider);
		addAndMakeVisible(m_compressorMixSliderLabel);

		m_compressorMixSliderAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "compressorMix", m_compressorMixSlider);

	}

	void resized() override {
		m_compressorMixSlider.setBounds(1390, 1000, 100, 100);
		m_compressorMixSliderLabel.setBounds(1320, 1000, 50, 100);
	}

private:
	juce::Label m_compressorMixSliderLabel;
	juce::Slider m_compressorMixSlider;

	using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

	std::unique_ptr<SliderAttachment> m_compressorMixSliderAttachment;
	
	GranularinfiniteAudioProcessor& audioProcessor;
};
