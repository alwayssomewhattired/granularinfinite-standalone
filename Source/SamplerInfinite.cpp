
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
#include <pybind11/embed.h>
namespace py = pybind11;


//==============================================================================
SamplerInfinite::SamplerInfinite(GranularinfiniteAudioProcessor& p)
    : audioProcessor(p),
    m_spotifyButton(buttonPalette.spotifyButton),
    m_sourceDownloadButton(buttonPalette.sourceDownloadButton)

{

    //spotify authentication
    m_auth = std::make_unique<SpotifyAuthenticator>();

    // find out if needed 
    setSize(1900, 1000);


    addAndMakeVisible(buttonPalette);
    addAndMakeVisible(m_spotifyButton);
    addAndMakeVisible(m_sourceDownloadButton);
    addAndMakeVisible(m_spotifyBrowser);


    spotifyButtonHandler();
    sourceDownloadHandler();

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
        std::cout << "the real token: " << m_spotifyAuthToken << "\n";
        m_spotifyAPI.setAccessToken(m_spotifyAuthToken);
        std::cout << "finisehd and authenticated\n";
        };
}

void SamplerInfinite::sourceDownloadHandler()
{
    m_sourceDownloadButton.onClick = [this] {
        spotifyFetcher = std::make_unique<SpotifyFetcher>(m_spotifyAuthToken);

        spotifyFetcher->onSongsFetched = [this](const juce::StringArray& songs)
            {
                if (songs.isEmpty())
                {
                    std::cout << "Received nothing from Spotify...\n";
                    return;
                }

                std::thread([songs]()
                    {
                        static std::unique_ptr<py::scoped_interpreter> guard;
                        if (!guard) {
                            guard = std::make_unique<py::scoped_interpreter>();
                            py::gil_scoped_acquire g;
                            PyEval_InitThreads();
                        }
                        std::cout << "getting this song: " << songs[2].toStdString() << "\n";
                        py::gil_scoped_acquire acquire;
                        py::module sys = py::module::import("sys");
                        sys.attr("path").attr("append")("C:/Users/zacha/Desktop/granularinfinite/Source");

                        auto result = runPythonFunction(songs[2]);
                        std::cout << "this is the result: " << result.toStdString() << "\n";
                    }).detach();

            };
        //// trigger this via button click in the future
        spotifyFetcher->startFetching();
        std::cout << "all done\n";
        };
}

//==============================================================================

void SamplerInfinite::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
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
    m_spotifyBrowser.setBounds(400, y, 1000, 800);
    m_spotifyButton.setBounds(200, y + 400, 80, 80);
    m_sourceDownloadButton.setBounds(1600, y + 400, 80, 80);
}