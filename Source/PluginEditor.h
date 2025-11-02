
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include "ButtonPalette.h"
#include "GranularInfinite.h"
#include "SamplerInfinite.h"

class GranularInfinite;
//==============================================================================
/**
*/
class GranularinfiniteAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    GranularinfiniteAudioProcessorEditor(GranularinfiniteAudioProcessor& p);

    ~GranularinfiniteAudioProcessorEditor() override;

    //==============================================================================

    
    void paint (juce::Graphics&) override;
    void resized() override;


private:
    GranularinfiniteAudioProcessor& audioProcessor;


    // using a pointer fixed this issue
    std::unique_ptr<GranularInfinite> granularPage;
    //SamplerInfinite samplerPage{ audioProcessor };
    std::unique_ptr<SamplerInfinite> samplerPage;

    ButtonPalette buttonPalette;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularinfiniteAudioProcessorEditor)
};
