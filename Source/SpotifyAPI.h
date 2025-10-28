
#pragma once
#include <JuceHeader.h>
#include "SpotifyBrowser.h"

class SpotifyAPI
{
public:
	SpotifyAPI(){}

	void setAccessToken(const juce::String& token)
	{
		m_accessToken = token;
	}

	void searchArtists(const juce::String& query, std::function<void(juce::Array<SpotifyItem>)> callback)
	{
		auto url = "https://api.spotify.com/v1/search?q=" + juce::URL::addEscapeChars(query, true)
			+ "&type=artist&limit=20";

		fetch(url, [callback](auto json) {
			juce::Array<SpotifyItem> results;
			auto artists = json["artists"]["items"];
			if (auto* artistArray = artists.getArray())
			{
				for (const auto& artist : *artistArray)
					results.add({ artist["id"].toString(), artist["name"].toString() });
				callback(results);
			}
			});
	}

	void getArtistsAlbum(const juce::String& artistId, std::function<void(juce::Array<SpotifyItem>)> callback)
	{
		auto url = "https://api.spotify.com/v1/artists/" + artistId + "/albums?limit=20";
		fetch(url, [callback](auto json) {
			juce::Array<SpotifyItem> results;
			auto items = json["items"];
			if (auto* itemArray = items.getArray())
			{
				for (const auto& album : *itemArray)
					results.add({ album["id"].toString(), album["name"].toString() });
				callback(results);
			}
			});
	}

	void getAlbumTracks(const juce::String& albumId, std::function<void(juce::Array<SpotifyItem>)> callback)
	{
		auto url = "https://api.spotify.com/v1/albums/" + albumId + "/tracks";
		fetch(url, [callback](auto json) {
			juce::Array<SpotifyItem> results;
			auto items = json["items"];
			if (auto* itemArray = items.getArray())
			{
				for (const auto&  track : *itemArray)
					results.add({ track["id"].toString(), track["name"].toString() });
				callback(results);
			}
			});
	}

private:
	juce::String m_accessToken;

		void fetch(const juce::String& url, std::function<void(juce::var)> callback)
	{
			juce::URL request(url);
			juce::String authHeader = "Bearer " + m_accessToken;

			auto* thread = new std::thread([=]() {
				std::unique_ptr<juce::WebInputStream> stream;

				try
				{
					stream = std::make_unique<juce::WebInputStream>(request, true);
					stream->withExtraHeaders("Authorization: " + authHeader);
					stream->withConnectionTimeout(5000);
					stream->connect(nullptr);
				}
				catch (...)
				{
					std::cout << "SpotifyAPI: exception during HTTP request\n";
					return;
				}

				if (stream != nullptr && stream->isError() == false)
				{
					auto jsonText = stream->readEntireStreamAsString();
					auto json = juce::JSON::parse(jsonText);
					if (!json.isVoid())
						callback(json);
				}

				stream.reset();
				});

			thread->detach();
	}
};
