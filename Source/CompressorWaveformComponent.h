
#pragma once
#include "JuceHeader.h"


class CompressorWaveformComponent : public juce::Component {

public:
	CompressorWaveformComponent(
		juce::AudioBuffer<float>& incomingBuffer,
		juce::AudioBuffer<float>& outputBuffer,
		juce::AbstractFifo& fifo);

	//void updateSamples(juce::AudioBuffer<float>& incomingBuffer,
	//	juce::AudioBuffer<float>& outputBuffer,
	//	juce::AbstractFifo& fifo);
	void updateCompressorLevels(const float& value);
	void paint(juce::Graphics&) override;

private:
	void drawCompressionWaveform(const juce::AudioBuffer<float>& buffer, juce::Graphics& g, juce::Rectangle<int> bounds);
	void drawCompressorLevels(const float value, juce::Graphics& g, juce::Rectangle<int> bounds);
	juce::AudioBuffer<float>& m_incomingBuffer;
	juce::AudioBuffer<float>& m_outputBuffer;
	juce::AbstractFifo& m_fifo;
	float m_thresholdLevel;
	juce::Rectangle<int> m_bounds{ 50,50,50,50 };
};