
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "../Sample.h"

class WaveformDisplay : public juce::Component
{
public:
    WaveformDisplay() : m_sample(nullptr){}

    void setBuffer(const juce::AudioBuffer<float>& audioBuffer)
    {
        if (audioBuffer.getNumChannels() == 0 || audioBuffer.getNumSamples() == 0)
        {
            buffer.setSize(0, 0);
            repaint();
            return;
        }
        buffer.makeCopyOf(audioBuffer);
        repaint();
    }

    void setSample(Sample* sample)
    {
        m_sample = sample;
    }

    juce::AudioBuffer<float>& getBuffer()
    {
        return buffer;
    }

    void setGrainArea(const float& grainArea)
    {
        auto bounds = getLocalBounds();
        m_grainAreaRect = bounds.removeFromLeft(static_cast<int>(grainArea));

        repaint();
    }

    void setPlayheadPosition()
    {
        if (!m_sample)
        {
            return;
        }
        double curr = m_sample->transportSource.getCurrentPosition();
        double total = m_sample->transportSource.getLengthInSeconds();

        float posNormalized = 0.0f; 
        if (total > 0.0f) 
            posNormalized = static_cast<float>(curr / total);

        auto bounds = getLocalBounds();
        float width = bounds.getWidth();
        float height = bounds.getHeight();

        float x = (float)m_grainAreaRect.getX() + posNormalized * width;
        float y = (float)m_grainAreaRect.getY();

        m_playheadPosition.setBounds(x, y, 2.0f, height);
        repaint();
    }

    void clear()
    {
        buffer.setSize(0, 0, false, false, false);
        repaint();
    }

	void paint(juce::Graphics& g) override
	{

        if (buffer.getNumSamples() == 0) {
            return;
        }
        g.setColour(juce::Colours::green);

        auto bounds = getLocalBounds();
        g.drawRect(bounds);
        int height = bounds.getHeight();
        int width = bounds.getWidth();

        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        // grain-area rectangle
        g.setColour(juce::Colours::grey.withAlpha(0.6f));
        g.fillRect(m_grainAreaRect);

        // playhead position
        if (m_sample)
        {
        g.setColour(juce::Colours::white);
        g.fillRect(m_playheadPosition);
        }

        // reads first channel
        // currently empty
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
    // possibly might not need this. replacing with 'sample' instead...
    juce::AudioBuffer<float> buffer;

    Sample* m_sample;

    juce::Rectangle<int> m_grainAreaRect;
    juce::Rectangle<float> m_playheadPosition;
};