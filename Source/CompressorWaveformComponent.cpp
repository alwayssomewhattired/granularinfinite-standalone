
#include "CompressorWaveformComponent.h"

CompressorWaveformComponent::CompressorWaveformComponent(
	juce::AudioBuffer<float>& incomingBuffer,
	juce::AudioBuffer<float>& outputBuffer,
	juce::AbstractFifo& fifo)
	: m_incomingBuffer(incomingBuffer), m_outputBuffer(outputBuffer), m_fifo(fifo) {
}

void CompressorWaveformComponent::drawCompressionWaveform(const juce::AudioBuffer<float>& buffer, juce::Graphics& g, juce::Rectangle<int> bounds) {
	const int numSamples = buffer.getNumSamples();
	if (numSamples == 0) return;

	juce::Path p;

	auto* data = buffer.getReadPointer(0);
	const float midY = bounds.getCentreY();
	const float height = (float)bounds.getHeight();

	float xOffset = bounds.getX();
	float yOffset = bounds.getY();

	p.startNewSubPath(
		xOffset + 0, 
		yOffset + midY - (data[0] * height * 0.5f));

	// Map samples horizontally across the width of the component
	for (int i = 1; i < numSamples; ++i) {
		float x = xOffset + juce::jmap<float>(i, 0, numSamples - 1, 0.0f, (float)bounds.getWidth());
		float y = yOffset + midY - (data[i] * height * 0.5f);

		p.lineTo(x, y);
	}

	g.strokePath(p, juce::PathStrokeType(1.5f));
}

void CompressorWaveformComponent::drawCompressorLevels(const float value, juce::Graphics& g, juce::Rectangle<int> bounds) {
	const float minDb = -60.0f;
	const float maxDb = 0.0f;

	float proportion = juce::jmap(value, minDb, maxDb, 1.0f, 0.0f);
	float y = bounds.getY() + proportion * bounds.getHeight();

	g.drawLine((float)bounds.getX(), y,
		(float)bounds.getRight(), y,
		2.0f);
}

//void CompressorWaveformComponent::updateSamples(juce::AudioBuffer<float>& incomingBuffer,
//	juce::AudioBuffer<float>& outputBuffer,
//	juce::AbstractFifo& fifo) {
//	//m_incomingBuffer = incomingBuffer;
//	//m_outputBuffer = outputBuffer;
//	//m_fifo = fifo;
//	//repaint();
//}

void CompressorWaveformComponent::updateCompressorLevels(const float& value) {
	m_thresholdLevel = value;
	repaint();
}

void CompressorWaveformComponent::paint(juce::Graphics& g) {

	g.fillAll(juce::Colours::black);

	g.setColour(juce::Colours::red.withAlpha(0.5f));
	drawCompressionWaveform(m_incomingBuffer, g, juce::Rectangle<int>(300, -70, 200, 500));

	g.setColour(juce::Colours::yellow.withAlpha(0.5f));
	drawCompressorLevels(m_thresholdLevel, g, m_bounds);

	g.setColour(juce::Colours::green.withAlpha(0.5f));
	drawCompressionWaveform(m_outputBuffer, g, juce::Rectangle<int>(50, -70, 200, 500));

}