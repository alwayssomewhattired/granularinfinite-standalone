
#pragma once

#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "ButtonPalette.h"
#include "SpotifyBrowser.h"
#include "SpotifyList.h"
#include "SpotifyAPI.h"
#include "SpotifyFetcher.h"
#include "SpotifyAuthenticator.h"
#include "SearchableComboBox.h"


//==============================================================================
/**
*/
class SamplerInfinite : public juce::Component
{
public:
    SamplerInfinite(GranularinfiniteAudioProcessor& p, ButtonPalette& bp);

    ~SamplerInfinite() override;

    //==============================================================================


    void paint(juce::Graphics&) override;
    void resized() override;

    std::function<void()> onSamplerComponentButtonClicked;


    void spotifyButtonHandler();
    void sampleProcessing();
    void sourceDownloadHandler();



private:

    struct Config {
        int chunkSize;
        int sampleRate;
        int channels;
        int productDurationSamples;
    };

    Config config;

    SearchableComboBox m_frequencyBox;
    SearchableComboBox m_directoryBox;

    ButtonPalette& buttonPalette;
    // make a class specifically for the grainPositionSlider.
    //  pass a reference of the audioProcessor object to the initializer list of the audioEditor
    //   access m_maxFileSize from that object and use it to set the sliders range
    //    the slider should now accurately depict values

    juce::TextButton componentButton;

    juce::TextButton& m_spotifyButton;
    juce::TextButton& m_sourceDownloadButton;

    std::unique_ptr<SpotifyAuthenticator> m_auth;
    juce::String m_spotifyAuthToken;

    // results for spotify
    //SpotifyList m_spotifyList;


    SpotifyAPI m_spotifyAPI;

    // text searcher for spotify
    SpotifyBrowser m_spotifyBrowser{ m_spotifyAPI };

    // selected song(s)
    juce::Rectangle<int> m_selected{ 1150, 110, 300, 780 };

    // directory component


    // unused
    std::vector<std::string> selectedFreqs;

    // samplerinfinite (get rid of this)
    std::unique_ptr<SpotifyFetcher> spotifyFetcher;

    GranularinfiniteAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerInfinite)
};
