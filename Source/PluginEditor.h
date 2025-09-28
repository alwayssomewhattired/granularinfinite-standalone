
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MyLookAndFeel.h"
#include "KeyToNote.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include "SampleLabel.h"
#include "KeyButton.h"
#include "NoteLabel.h"
#include "ButtonPalette.h"
#include "BiMap.h"
#include "CustomLookAndFeel.h"
#include "DualThumbSlider.h"

//==============================================================================
/**
*/
class GranularinfiniteAudioProcessorEditor  : public juce::AudioProcessorEditor,
    public juce::KeyListener
{
public:
    GranularinfiniteAudioProcessorEditor(GranularinfiniteAudioProcessor& p);

    ~GranularinfiniteAudioProcessorEditor() override;

    //==============================================================================

    
    void paint (juce::Graphics&) override;
    void resized() override;

    bool keyPressed(const juce::KeyPress& key, Component* originatingComponent) override;
    bool keyStateChanged(bool isKeyDown, Component* originatingComponent) override;

    void octaveUp(juce::TextButton& button);
    void octaveDown(juce::TextButton& button);

    void synthToggleHandler(juce::TextButton& button);
    void grainLengthSliderHandler();
    void sampleLabelHandler(SampleLabel& button);


private:

    // LOOK AND FEEL
    CustomLookAndFeel customLook;
    DiySlider diySlider;

    juce::String synthNote = "G4";

    std::map<juce::String, std::unique_ptr<juce::File>> noteToFile;
    std::set<char> currentlyPressedKeys;
    juce::OwnedArray<KeyButton> keyButtons;
    juce::OwnedArray<SampleLabel> sampleLabels;
    juce::OwnedArray<NoteLabel> noteLabels;
    std::map<char, juce::String> keyToNote;
    BiMap<juce::String, juce::String> noteToSample;
    juce::MidiBuffer pendingMidi;
    int octave = 4;
    juce::String currentlyPressedSample = "none";
    ButtonPalette buttonPalette;

    juce::Slider* grainAmountControl;
    DualThumbSlider grainLengthSlider;


    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    std::unique_ptr<SliderAttachment> grainAmountAttachment;
    std::unique_ptr<SliderAttachment> grainMaxLength;

    GranularinfiniteAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularinfiniteAudioProcessorEditor)
};
