
#include "CompressorWaveformComponent.h"

CompressorWaveformComponent::CompressorWaveformComponent(
	juce::AudioBuffer<float>& outputBuffer)
	: m_outputBuffer(outputBuffer){
}

void CompressorWaveformComponent::drawCompressionWaveform(const juce::AudioBuffer<float>& buffer, juce::Graphics& g, juce::Rectangle<int> bounds) {
	const int numSamples = buffer.getNumSamples();
	if (numSamples == 0) return;

	juce::Path p;

	auto* data = buffer.getReadPointer(0);
	if (!data) return;

	const float midY = bounds.getY() + bounds.getHeight() * 0.5f;
	const float amplitudeScale = bounds.getHeight() * 0.5f;
	const float height = (float)bounds.getHeight();

	float xOffset = bounds.getX();
	float yOffset = bounds.getY();

	p.startNewSubPath(
		xOffset, 
		yOffset + midY - (data[0] * height * 0.5f));

	// Map samples horizontally across the width of the component
	for (int i = 1; i < numSamples; ++i) {
		float x = xOffset + juce::jmap<float>(i, 0, numSamples - 1, 0.0f, (float)bounds.getWidth());

		float sample = data[i];
		float unipolar = std::abs(sample);
   
		float y = yOffset + height * (1.0f - unipolar); 


		p.lineTo(x, y);
	}

	g.strokePath(p, juce::PathStrokeType(1.5f));
}


void CompressorWaveformComponent::updateCompressorThreshold(const float& threshold) {
	m_thresholdLevel = threshold;
	repaint();
}
void CompressorWaveformComponent::updateCompressorRatio(const float& ratio) {

	m_ratioLevel = (ratio - 1.0f) / (20.0f - 1.0f); // this is diy normalized
	repaint();
}

void CompressorWaveformComponent::drawCompressorOverlay(juce::Graphics& g, juce::Rectangle<int> bounds, float threshold, float ratio) {
	const float midY = bounds.getCentreY();
	const float height = (float)bounds.getHeight();
	const int numSamples = m_outputBuffer.getNumSamples();
	const float* data = m_outputBuffer.getReadPointer(0);

	// here we draw the theshold line
	float yThreshold = midY - (threshold * height * 0.5f);

	g.setColour(juce::Colours::white.withAlpha(0.5f));
	g.drawLine((float)bounds.getX(), yThreshold, (float)bounds.getRight(), yThreshold, 1.5f);

	// this draws gain-reduction shading (for samples above threshold)
	g.setColour(juce::Colours::red.withAlpha(0.25f));

	for (int i = 0; i < numSamples; ++i) {
		float sample = std::abs(data[i]);

		if (sample > threshold) {
			float x = juce::jmap<float>(i, 0, numSamples - 1, (float)bounds.getX(), (float)bounds.getRight());

			float ySample = midY - (sample * height * 0.5f);

			g.drawLine(x, yThreshold, x, ySample, 1.0f);
		}
	}
}

void CompressorWaveformComponent::paint(juce::Graphics& g) {

	g.fillAll(juce::Colours::black);


	g.setColour(juce::Colours::green.withAlpha(0.5f));
	drawCompressionWaveform(m_outputBuffer, g, m_bounds);

	// this draws threshold + gain reduction shading
	drawCompressorOverlay(g, m_bounds, m_thresholdLevel, m_ratioLevel);

}