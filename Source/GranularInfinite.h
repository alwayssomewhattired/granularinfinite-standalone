
#pragma once

#include <JuceHeader.h>
//#include "PluginProcessor.h"
#include "MyLookAndFeel.h"
#include "KeyToNote.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include "components/SampleLabel.h"
#include "components/KeyButton.h"
#include "components/NoteLabel.h"
#include "components/ButtonPalette.h"
#include "BiMap.h"
#include "CustomLookAndFeel.h"
#include "components/DualThumbSlider.h"
#include "components/GrainPositionControl.h"
#include "components/WaveformDisplay.h"
#include "components/KeyButtonMods.h"
#include "components/CompressorWaveformComponent.h"
#include "components/ScrollableList.h"
#include "sourceinfinite/SpotifyBrowser.h"
#include "sourceinfinite/SpotifyList.h"
#include "sourceinfinite/SpotifyAPI.h"
#include "sourceinfinite/SpotifyFetcher.h"

class GranularinfiniteAudioProcessor;
//==============================================================================
/**
*/

class GranularInfinite : public juce::Component, public juce::KeyListener, public juce::ChangeListener, private juce::Timer

{
public:
    GranularInfinite(GranularinfiniteAudioProcessor& p, ButtonPalette& bp);

    ~GranularInfinite() override;

    //==============================================================================


    void paint(juce::Graphics&) override;
    void resized() override;


    bool keyPressed(const juce::KeyPress& key, Component* originatingComponent) override;
    bool keyStateChanged(bool isKeyDown, Component* originatingComponent) override;

    void sampleRefresh(KeyButton* button);

    void octaveUp(juce::TextButton& button);
    void octaveDown(juce::TextButton& button);

    std::function<void()> onComponentButtonClicked;

    //void spotifyButtonHandler();
    //void sourceDownloadHandler();

    void synthToggleHandler(juce::TextButton& button);
    void grainLengthSliderHandler();
    void grainAreaSliderHandler();
    void playheadPositionHandler();
    void sampleLabelHandler(SampleLabel& button);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void compressorWaveformHandle();
    void globalGainSliderHandler(juce::Slider& slider);

private:
    bool m_isCompressorTimer;
    bool m_isPlayhead;
    void compressorWaveformTimer();

    juce::AudioProcessorValueTreeState& m_apvts;

    std::unique_ptr<CompressorWaveformComponent> m_compressorWaveformComponent = nullptr;

    // LOOK AND FEEL
    CustomLookAndFeel customLook;
    DiySlider diySlider;

    juce::String synthNote = "G4";

    const std::vector<std::string> noteRange = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    std::map<juce::String, std::unique_ptr<juce::File>> noteToFile;
    std::set<char> currentlyPressedKeys;
    // i believe these are the keybuttons that are in current octave range
    juce::OwnedArray<KeyButton> keyButtons;
    juce::OwnedArray<SampleLabel> sampleLabels;
    juce::OwnedArray<NoteLabel> noteLabels;
    std::map<char, juce::String> keyToNote;
    BiMap<juce::String, juce::String> noteToSample;
    const std::map<std::string, double>& m_noteToFreq = createNoteToFreq();

    juce::MidiBuffer pendingMidi;
    int octave = 3;
    juce::String currentlyPressedSample = "none";

    void timerCallback() override;

    KeyButtonMods m_keyButtonMods;

    ButtonPalette& buttonPalette;
    // make a class specifically for the grainPositionSlider.
    //  pass a reference of the audioProcessor object to the initializer list of the audioEditor
    //   access m_maxFileSize from that object and use it to set the sliders range
    //    the slider should now accurately depict values

    juce::TextButton componentButton;

    juce::Label& grainSpacingLabel;
    juce::Slider& grainSpacingSlider;

    juce::Label& grainAmountLabel;
    juce::Slider& grainAmountSlider;

    juce::Label& grainLengthLabel;
    DualThumbSlider grainLengthSlider;

    juce::Label& grainAreaLabel;
    DualThumbSlider grainAreaSlider;

    std::unique_ptr<juce::TextButton> hanningToggleButton;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> grainSpacingAttachment;
    std::unique_ptr<SliderAttachment> grainAmountAttachment;
    std::vector<std::unique_ptr<SliderAttachment>> frequencyUpwardCompressorAttachments;

    std::unique_ptr<ButtonAttachment> hanningToggleAttachment;
    std::unique_ptr<SliderAttachment> globalGainAttachment;
    std::unique_ptr<SliderAttachment> compressorThresholdAttachment;
    std::unique_ptr<SliderAttachment> compressorRatioAttachment;
    std::unique_ptr<SliderAttachment> compressorAttackCoeffAttachment;
    std::unique_ptr<SliderAttachment> compressorReleaseCoeffAttachment;
    std::unique_ptr<SliderAttachment> compressorGainAttachment;

    // dunno if this is still needed v
    std::unique_ptr<SliderAttachment> grainMaxLength;

    WaveformDisplay m_waveformDisplay;
    ScrollableList m_scrollableList;


    GranularinfiniteAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GranularInfinite)
};
