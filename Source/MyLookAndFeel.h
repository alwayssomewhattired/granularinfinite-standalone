// MyLookAndFeel.h
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class MyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MyLookAndFeel()
    {
    }

    // === Keys (.key, .active, .assigned) ===
    void drawButtonBackground(juce::Graphics& g,
        juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        // Base background (like .key)
        juce::Colour base = juce::Colours::white;
        g.setColour(base);
        g.fillRoundedRectangle(bounds, 4.0f);

        // Border (.assigned vs default)
        bool isAssigned = button.getToggleState(); // treat toggle as "assigned"
        g.setColour(isAssigned ? juce::Colours::green : juce::Colours::white);
        g.drawRoundedRectangle(bounds, 4.0f, 2.0f);

        // Active (.active)
        if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        {
            g.setColour(juce::Colours::lightblue);
            g.fillRoundedRectangle(bounds, 4.0f);
        }
    }

    // === Key text (.key-name, .sample-name) ===
    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
        bool /*isMouseOver*/, bool /*isButtonDown*/) override
    {
        g.setColour(juce::Colours::black);
        g.setFont(juce::Font(20.0f, juce::Font::bold));
        g.drawText(button.getButtonText(), button.getLocalBounds(),
            juce::Justification::centred, true);
    }

    // === Labels (e.g. .sample-name styling) ===
    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {

        setColour(juce::Label::textColourId, juce::Colour::fromHSV(119.1f, 100.0f, 100.0f, 1.0f));
        g.setFont(juce::Font(10.0f)); // .sample-name font-size

        g.drawText(label.getText(), label.getLocalBounds(),
            juce::Justification::centred, true);
    }
};
