#include "PluginProcessor.h"
//#include "PluginEditor.h"
#include "GranularInfinite.h"
#include "OpenConsole.h"
#include "ButtonPalette.h"



//==============================================================================
GranularinfiniteAudioProcessorEditor::GranularinfiniteAudioProcessorEditor
(GranularinfiniteAudioProcessor& p)
    : juce::AudioProcessorEditor(&p),
    audioProcessor(p)

{
    openConsole();
    granularPage = std::make_unique<GranularInfinite>(audioProcessor, buttonPalette);
    samplerPage = std::make_unique<SamplerInfinite>(audioProcessor, buttonPalette);
    setSize(1900, 1000);

    addAndMakeVisible(*granularPage);
    addAndMakeVisible(*samplerPage);
    samplerPage->setVisible(false);

    granularPage->onComponentButtonClicked = [this]() {
        granularPage->setVisible(false);
        samplerPage->setVisible(true);
        };

    samplerPage->onSamplerComponentButtonClicked = [this]() {
        std::cout << "r we in?\n";
        samplerPage->setVisible(false);
        granularPage->setVisible(true);
        };

}

GranularinfiniteAudioProcessorEditor::~GranularinfiniteAudioProcessorEditor()
{

}

//==============================================================================






//==============================================================================


void GranularinfiniteAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void GranularinfiniteAudioProcessorEditor::resized()
// when you get the time, for the love of god refactor using flexbox
{


    int x = 300;                  
    int y = 100;                  
    int buttonWidth = 60;
    int buttonHeight = 120;
    int labelHeight = 20;
    int spacing = 5;


    buttonPalette.setBounds(getLocalBounds());
    if (granularPage == nullptr)
        std::cout << "your null!!!\n";
    granularPage->setBounds(getLocalBounds());
    samplerPage->setBounds(getLocalBounds());
}

