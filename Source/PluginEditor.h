
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
#include "GrainPositionControl.h"
#include "WaveformDisplay.h"

//==============================================================================
/**
*/
class GranularinfiniteAudioProcessorEditor  : public juce::AudioProcessorEditor,
    public juce::KeyListener, public juce::ChangeListener, private juce::Timer
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
    void grainPositionSliderHandler();
    void playheadPositionHandler();
    void sampleLabelHandler(SampleLabel& button);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;


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

    void timerCallback() override;

    ButtonPalette buttonPalette;
    // make a class specifically for the grainPositionSlider.
    //  pass a reference of the audioProcessor object to the initializer list of the audioEditor
    //   access m_maxFileSize from that object and use it to set the sliders range
    //    the slider should now accurately depict values

    juce::Label& grainSpacingLabel;
    juce::Slider& grainSpacingSlider;

    juce::Label& grainAmountLabel;
    juce::Slider& grainAmountSlider;

    juce::Label& grainLengthLabel;
    DualThumbSlider grainLengthSlider;

    juce::Label& grainPositionLabel;
    GrainPositionControl grainPositionSlider;



    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    std::unique_ptr<SliderAttachment> grainSpacingAttachment;
    std::unique_ptr<SliderAttachment> grainAmountAttachment;
    std::unique_ptr<SliderAttachment> grainPositionAttachment;

    // dunno if this is still needed v
    std::unique_ptr<SliderAttachment> grainMaxLength;

    WaveformDisplay m_waveformDisplay;

    GranularinfiniteAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularinfiniteAudioProcessorEditor)
};
