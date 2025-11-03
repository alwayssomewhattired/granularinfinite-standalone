#pragma once

#include "SamplerInfinite.h"
#include "ButtonPalette.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PythonSoulseek.h"
#include "SpotifyBrowser.h"
#include "SpotifyList.h"
#include "SpotifyAPI.h"
#include "SpotifyFetcher.h"
#include "SpotifyAuthenticator.h"
#include "KeyToNote.h"
#include <pybind11/embed.h>
namespace py = pybind11;


//==============================================================================
SamplerInfinite::SamplerInfinite(GranularinfiniteAudioProcessor& p, ButtonPalette& bp)
    : audioProcessor(p),
    buttonPalette(bp),
    m_spotifyButton(buttonPalette.spotifyButton),
    m_sourceDownloadButton(buttonPalette.sourceDownloadButton)
{

    //spotify authentication
    m_auth = std::make_unique<SpotifyAuthenticator>();

    // find out if needed 
    setSize(1900, 1000);

    //juce::TextButton& componentButton = buttonPalette.componentButton;

    componentButton.setButtonText(buttonPalette.componentButton.getButtonText());
    componentButton.setColour(
        juce::TextButton::buttonColourId,
        buttonPalette.componentButton.findColour(juce::TextButton::buttonColourId)
    );

    addAndMakeVisible(buttonPalette);
    addAndMakeVisible(componentButton);
    addAndMakeVisible(m_spotifyButton);
    addAndMakeVisible(m_sourceDownloadButton);
    addAndMakeVisible(m_spotifyBrowser);
    addAndMakeVisible(m_frequencyBox);

    // handler

    spotifyButtonHandler();
    sourceDownloadHandler();

    componentButton.onClick = [this]() {
            if (onSamplerComponentButtonClicked)
                onSamplerComponentButtonClicked();
        };

    m_spotifyBrowser.onTrackClicked = [this]() {
        repaint();
        };

    m_frequencyBox.onItemSelected = [](juce::String selected) {
        std::cout << "selected: " << selected << "\n";
        };

    Config config{
        8192,       // chunkSize
        44100,      // sampleRate
        1,          // channels
        96000       // productDurationSamples
    };


}

SamplerInfinite::~SamplerInfinite()
{
    spotifyFetcher->stopFetching();
}

//==============================================================================


void SamplerInfinite::spotifyButtonHandler()
{
    m_spotifyButton.onClick = [this] {

        m_auth->init("8df0570e51ae419baf4a7e2845a43cb4", "5aae9f994086437696de02533fd96ebd", "http://127.0.0.1:8888/callback");
        m_auth->startAuthentication();
        m_spotifyAuthToken = m_auth->waitAndGetToken();
        m_spotifyAPI.setAccessToken(m_spotifyAuthToken);
        };
}

void SamplerInfinite::sourceDownloadHandler()
{
    m_sourceDownloadButton.onClick = [this] {

        // make this work with the map from spotifybrowser

        std::map<juce::String, juce::String>& songs = m_spotifyBrowser.getSelectedMap();

        std::thread([songs]()
            {
                static std::unique_ptr<py::scoped_interpreter> guard;
                if (!guard) {
                    guard = std::make_unique<py::scoped_interpreter>();
                    py::gil_scoped_acquire g;
                    PyEval_InitThreads();
                }
                //std::cout << "getting this song: " << songs[2].toStdString() << "\n";
                py::gil_scoped_acquire acquire;
                py::module sys = py::module::import("sys");
                sys.attr("path").attr("append")("C:/Users/zacha/Desktop/granularinfinite/Source");

                for (const auto& [k, v] : songs)
                {
                    auto result = runPythonFunction(v);
                    std::cout << "this is the result: " << result.toStdString() << "\n";
                }
            }).detach();

    };
}

//==============================================================================

void SamplerInfinite::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::green);
    g.drawRect(m_selected, 1);

    g.setFont(16.0f);
    int textY = m_selected.getY() + 20;

    if (m_spotifyBrowser.getSelectedMap().empty())
    {
        return;
    }

    std::map<juce::String, juce::String>& map(m_spotifyBrowser.getSelectedMap());
    if (!map.empty())
    {
        for (const auto& [k, v] : map)
        {
            if (textY < m_selected.getBottom())
            {
                g.drawText(
                    k,
                    m_selected.getX() + 10,
                    textY,
                    m_selected.getWidth() - 20,
                    20,
                    juce::Justification::centredLeft,
                    false
                );
                textY += 22;
            }
            else break;
        }
    }
}

void SamplerInfinite::resized()
// when you get the time, for the love of god refactor using flexbox
{

    int x = 300;
    int y = 100;
    int buttonWidth = 60;
    int buttonHeight = 120;
    int labelHeight = 20;
    int spacing = 5;

    buttonPalette.setBounds(getLocalBounds());
    componentButton.setBounds(1600, y + 400, 80, 80);
    m_spotifyBrowser.setBounds(150, y, 1000, 800);
    m_spotifyButton.setBounds(50, y + 400, 80, 80);
    m_sourceDownloadButton.setBounds(1500, y + 400, 80, 80);
    m_frequencyBox.setBounds(getLocalBounds());
}