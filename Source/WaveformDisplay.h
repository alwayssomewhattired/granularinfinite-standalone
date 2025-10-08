
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "PluginProcessor.h"

class WaveformDisplay : public juce::Component
{
public:
    WaveformDisplay() = default;

    void setBuffer(const juce::AudioBuffer<float>& audioBuffer)
    {
        buffer.makeCopyOf(audioBuffer);
        repaint();
    }

	void paint(juce::Graphics& g) override
	{
        if (buffer.getNumSamples() == 0)
            return;
        g.setColour(juce::Colours::green);

        auto bounds = getLocalBounds();
        g.drawRect(bounds);
        int height = bounds.getHeight();
        int width = bounds.getWidth();

        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        // reads first channel
        const float* channelData = buffer.getReadPointer(0);

        juce::Path path;
        path.startNewSubPath(0, height * 0.5f);

        // downsample to one pixel per column
        for (int x = 0; x < width; ++x)
        {
            int sampleIndex = juce::jmap<int>(x, 0, (int)width, 0, numSamples);
            float level = channelData[sampleIndex];
            float y = juce::jmap(level, -1.0f, 1.0f, (float)height, 0.0f);
            path.lineTo((float)x, y);
        }
        g.strokePath(path, juce::PathStrokeType(1.0f));
	}

private:
    juce::AudioBuffer<float> buffer;
};