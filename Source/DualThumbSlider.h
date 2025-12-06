#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class DualThumbSlider : public juce::Slider
{
public:
    DualThumbSlider(std::tuple<double, double, double> range)
    {
        setRange(std::get<0>(range), std::get<1>(range), std::get<2>(range));
        minValue = std::get<0>(range);
        maxValue = std::get<1>(range);
    }

    enum DragType { DragNone, DragMin, DragMax, DragMid };

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto minPos = valueToProportionOfLength(minValue);
        auto maxPos = valueToProportionOfLength(maxValue);

        float minX = bounds.getX() + bounds.getWidth() * minPos;
        float maxX = bounds.getX() + bounds.getWidth() * maxPos;
        float midX = (minX + maxX) * 0.5f;

        // Track
        g.setColour(juce::Colours::darkgrey);
        g.fillRect(bounds.reduced(0, bounds.getHeight() / 2.25f));

        // Highlight range
        g.setColour(juce::Colours::green);
        g.fillRect(juce::Rectangle<float>(minX, bounds.getCentreY() - 4, maxX - minX, 8));

        // Draw thumbs
        g.setColour(juce::Colours::white);
        drawThumb(g, minX, false);
        drawThumb(g, maxX, false);
        drawThumb(g, midX, true);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        auto bounds = getLocalBounds().toFloat();
        float x = (float)e.position.x;

        float minX = bounds.getWidth() * valueToProportionOfLength(minValue);
        float maxX = bounds.getWidth() * valueToProportionOfLength(maxValue);
        float midX = (minX + maxX) * 0.5f;

        float distMin = std::abs(x - minX);
        float distMax = std::abs(x - maxX);
        float distMid = std::abs(x - midX);

        float nearest = std::min({ distMin, distMax, distMid });

        if (nearest == distMin) dragging = DragMin;
        else if (nearest == distMax) dragging = DragMax;
        else dragging = DragMid;

        originalMin = minValue;
        originalMax = maxValue;
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        double newVal = proportionOfLengthToValue(e.position.x / getWidth());

        switch (dragging)
        {
        case DragMin:
            minValue = juce::jlimit(getMinimum(), maxValue, newVal);
            break;

        case DragMax:
            maxValue = juce::jlimit(minValue, getMaximum(), newVal);
            break;

        case DragMid:
        {
            double range = originalMax - originalMin;
            double center = newVal;

            double newMin = center - range * 0.5;
            double newMax = center + range * 0.5;

            // Clamp without changing width
            double offset = 0.0;

            if (newMin < getMinimum())
                offset = getMinimum() - newMin;
            else if (newMax > getMaximum())
                offset = getMaximum() - newMax;

            minValue = newMin + offset;
            maxValue = newMax + offset;
            break;
        }
        }

        repaint();
        if (onRangeChange) onRangeChange();
    }

    double getMinValue() const { return minValue; }
    double getMaxValue() const { return maxValue; }

    std::function<void()> onRangeChange;

private:
    void drawThumb(juce::Graphics& g, float x, bool ifMid)
    {
        if (ifMid) g.fillEllipse(x - 6.0f, getHeight() / 2.0f + 8.0f, 6.0f, 20.0f);
        else 
            g.fillEllipse(x - 6.0f, getHeight() / 2.0f - 6.0f, 12.0f, 12.0f);
    }

    double minValue = 25.0;
    double maxValue = 75.0;
    double originalMin = 25.0;
    double originalMax = 75.0;

    DragType dragging = DragNone;
};
