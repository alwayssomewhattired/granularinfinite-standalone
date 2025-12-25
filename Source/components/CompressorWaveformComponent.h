
#pragma once
#include "JuceHeader.h"


class CompressorWaveformComponent : public juce::Component {

public:
	CompressorWaveformComponent(
		juce::AudioBuffer<float>& outputBuffer);

	void updateCompressorThreshold(const float& threshold);
	void updateCompressorRatio(const float& ratio);
	void paint(juce::Graphics&) override;

private:
	void drawCompressionWaveform(const juce::AudioBuffer<float>& buffer, juce::Graphics& g, juce::Rectangle<int> bounds);
	void drawCompressorOverlay(juce::Graphics& g, juce::Rectangle<int> bounds, float threshold, float ratio);
	juce::AudioBuffer<float>& m_outputBuffer;
	float m_thresholdLevel = 0.1f;
	float m_ratioLevel = 0.0f;
	juce::Rectangle<int> m_bounds{ 0, 0, 500, 200 };
};