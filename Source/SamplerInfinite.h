
#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

class SamplerInfinite : private juce::Thread
{
public:
	SamplerInfinite(const juce::String& token)
		: juce::Thread("SpotifyAPIFetcher"),
		authToken(token)
	{
	}

	~SamplerInfinite() override
	{
		signalThreadShouldExit();
		stopThread(2000);
	}

	// Starts the background thread
	void startFetching()
	{
		if (!isThreadRunning())
			startThread();
	}

	// Stops the background thread
	void stopFetching()
	{
		signalThreadShouldExit();
	}

	// the main thread loop
	void run() override
	{
		// make this triggered by button click vvv
		while (!threadShouldExit())
		{
			fetchSongNames();
			wait(60000); // temporary, 60 second wait
		}
	}

	// callback function that plugin can assign
	std::function<void(const juce::StringArray&)> onSongsFetched;

	void fetchSongNames()
	{
		// obviously make url dynamic. currently hardcoded for top 10 songs
		juce::URL url("https://api.spotify.com/v1/me/top/tracks?limit=10");

		juce::String extraHeaders = "Authorization: Bearer " + authToken + "\r\nAccept: application / json\r\n";

		auto stream = url.createInputStream(false, nullptr, nullptr, extraHeaders, 10000);


		juce::String response;

		if (stream != nullptr)
		{
			response = stream->readEntireStreamAsString();
		}
		else
		{
			std::cout << "failed to connect... \n";
			return;
		}


		if (response.isNotEmpty())
		{
			juce::var json = juce::JSON::parse(response);

			if (json.isObject())
			{
				juce::StringArray names;
				auto items = json["items"];

				if (items.isArray())
				{
					for (auto& item : *items.getArray())
					{
						if (item.isObject())
						{
							names.add(item["name"].toString());
						}
					}

					if (onSongsFetched)
						onSongsFetched(names);
				}
			}
		}
	}

private:

	// token for spotify api access
	juce::String authToken;
};