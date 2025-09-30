#include <juce_gui_basics/juce_gui_basics.h>
#pragma once

class DualThumbSlider : public juce::Slider
{
public:
	DualThumbSlider(std::tuple<double, double, double> range)
	{
		double min = std::get<0>(range);
		double max = std::get<1>(range);
		double interval = std::get<2>(range);

		setRange(min, max, interval);

		minValue = min;
		maxValue = max;

	}

	void paint(juce::Graphics& g) override
	{
		auto bounds = getLocalBounds().toFloat();
		auto minPos = valueToProportionOfLength(minValue);
		auto maxPos = valueToProportionOfLength(maxValue);

		auto minX = bounds.getX() + bounds.getWidth() * minPos;
		auto maxX = bounds.getX() + bounds.getWidth() * maxPos;

		// Track
		g.setColour(juce::Colours::darkgrey);
		g.fillRect(bounds.reduced(0, bounds.getHeight() / 2.25));

		// Range highlight
		g.setColour(juce::Colours::green);
		g.fillRect(juce::Rectangle<float>(minX, bounds.getCentreY() - 4, maxX - minX, 8));

		// Thumbs
		g.setColour(juce::Colours::white);
		g.fillEllipse(minX, bounds.getCentreY() - 6, 12, 12);
		g.fillEllipse(maxX - 12, bounds.getCentreY() - 6, 12, 12);
	}

	void mouseDown(const juce::MouseEvent& e) override
	{
		draggingMin = std::abs(proportionOfLengthToValue(e.position.x / getWidth()) - minValue)
			< std::abs(proportionOfLengthToValue(e.position.x / getWidth()) - maxValue);
	}

	std::function<void()> onRangeChange;

	// works
	void mouseDrag(const juce::MouseEvent& e) override
	{


		double newValue = proportionOfLengthToValue(e.position.x / getWidth());
		if (draggingMin)
			minValue = juce::jlimit(getMinimum(), maxValue, newValue);
		else
			maxValue = juce::jlimit(minValue, getMaximum(), newValue);

		repaint();

		if (onRangeChange) {
			onRangeChange();
		}
	}

	double getMinValue() const { return minValue; }
	double getMaxValue() const { return maxValue; }

private:
	double minValue = 25.0;
	double maxValue = 75.0;
	bool draggingMin = true;
};