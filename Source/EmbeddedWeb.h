
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

class BrowserWindow : public juce::Component
{
public:
	BrowserWindow()
	{
		addAndMakeVisible(browser);
		std::cout << "browser deployed\n";
	}

	~BrowserWindow()
	{
		std::cout << "browser destroyed\n";
	}

	void setBrowser(const juce::String& url)
	{
		browser.goToURL(url);
		setVisible(true);
		resized();
		std::cout << "browser set\n";
	}

	void closeBrowser()
	{
		setVisible(false);
	}

	void resized() override
	{
		browser.setBounds(getLocalBounds());
	}

private:
	juce::WebBrowserComponent browser;
};