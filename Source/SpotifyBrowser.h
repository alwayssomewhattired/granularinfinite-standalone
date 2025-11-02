
#pragma once
#include <JuceHeader.h>
#include "SpotifyAPI.h"
#include "SpotifyList.h"

class SpotifyBrowser : public juce::Component,
	public juce::TextEditor::Listener
{
public:
	SpotifyBrowser(SpotifyAPI& api) : m_api(api)
	{
		searchBox.setTextToShowWhenEmpty("Search artists...", juce::Colours::green);
		searchBox.addListener(this);
		addAndMakeVisible(searchBox);

		addAndMakeVisible(artistList);
		addAndMakeVisible(albumList);
		addAndMakeVisible(trackList);

		artistList.onItemSelected = [this](const SpotifyItem& artist)
			{
				m_api.getArtistsAlbum(artist.id, [this](const juce::Array<SpotifyItem>& albums)
					{
						juce::MessageManager::callAsync([this, albums]() {
							albumList.updateItems(albums);
							trackList.clearItems();
						});
					});
			};

		albumList.onItemSelected = [this](const SpotifyItem& album)
			{
				m_api.getAlbumTracks(album.id, [this](const juce::Array<SpotifyItem>& tracks)
					{
						juce::MessageManager::callAsync([this, tracks]() {
							trackList.updateItems(tracks);
						});
					});
			};
		trackList.onItemSelected = [this](const SpotifyItem& track)
			{
				selectedMap[track.name] = track.id;
				// make editor repaint
				if (auto* parent = getParentComponent())
					parent->repaint();

			};
	}

	void resized() override
	{
		auto area = getLocalBounds().reduced(10);
		searchBox.setBounds(area.removeFromTop(30).reduced(0, 5));

		auto row = area;
		auto columnWidth = row.getWidth() / 3;
		artistList.setBounds(row.removeFromLeft(columnWidth).reduced(5));
		albumList.setBounds(row.removeFromLeft(columnWidth).reduced(5));
		trackList.setBounds(row.reduced(5));
	}

	void textEditorReturnKeyPressed(juce::TextEditor& editor) override
	{
		auto query = editor.getText();
		m_api.searchArtists(query, [this](const juce::Array<SpotifyItem>& artists)
			{
				juce::MessageManager::callAsync([this, artists]() {
					artistList.updateItems(artists);
					albumList.clearItems();
					trackList.clearItems();
					});
			});
	}

	std::map<juce::String, juce::String>& getSelectedMap()
	{
		return selectedMap;
	}

	std::function<void()> onTrackClicked;


private:
	juce::TextEditor searchBox;
	SpotifyList artistList{ "artists" };
	SpotifyList albumList{ "albums" };
	SpotifyList trackList{ "tracks" };

	std::map<juce::String, juce::String> selectedMap;

	// handles HTTP requests
	SpotifyAPI& m_api;
};