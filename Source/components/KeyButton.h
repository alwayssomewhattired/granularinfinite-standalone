
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <functional>
#include "PluginEditor.h"
#include "KeyToNote.h"
#include "Constants.h"

struct KeyButton : public juce::TextButton, public juce::FileDragAndDropTarget {
	using FileDroppedCallback = std::function<void(std::map<juce::String, juce::Array<juce::File>>& noteToFiles, const bool& isDir)>;

	KeyButton() {
		setRepaintsOnMouseActivity(true);
	}

	void setOnFileDropped(FileDroppedCallback cb) {
		onFileDropped = std::move(cb);
	}

	bool isInterestedInFileDrag(const juce::StringArray& files) override
	{
		return true;
	}

	void collectAudioFiles(const juce::File& file, juce::Array<juce::File>& results, juce::String fileFreq) {
		if (!file.exists()) return;

		static juce::String currentFreq;

		if (file.isDirectory()) {
			juce::Array<juce::File> children;
			file.findChildFiles(children, juce::File::findFilesAndDirectories, false);
			if (CreateNoteToMidi.find(file.getFileName()) != CreateNoteToMidi.end())
				currentFreq = file.getFileName();

			for (auto& child : children) collectAudioFiles(child, results, fileFreq);
		}
		else {
			if (file.hasFileExtension("wav;mp3;aiff;flac;ogg")) {
				m_noteToFiles[currentFreq].add(file);
			}
		}
	}

	void filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/) override
	{
		if (files.isEmpty()) return;

		juce::Array<juce::File> audioFiles;
		juce::String fileFreq;
		juce::File file;

		for (auto& path : files) {
			file = juce::File(path);
			if (file.isDirectory())
				m_isDir = true;
			else
				m_isDir = false;
			collectAudioFiles(file, audioFiles, fileFreq);
		}

		if (onFileDropped)
			onFileDropped(m_noteToFiles, m_isDir);
	}

	void setNoteName(const juce::String& noteName) {
		m_noteName = noteName;
	}

	const juce::String& getNoteName() {
		return m_noteName;
	}

	void setPosition(const int& pos) {
		m_position = pos;
	}

	const int& getPosition() {
		return m_position;
	}

private:

	FileDroppedCallback onFileDropped;
	bool m_isDir;

	// Key: frequency
	// Value: folder of audio files
	std::map<juce::String, juce::Array<juce::File>> m_noteToFiles;
	juce::String m_keyName;
	juce::String m_noteName;
	juce::String m_keyNameWithOctave;
	int m_position;
	juce::String assignedFile;

	juce::String trimmedFileName;

	juce::String sampleName;
};

class KeyButtons : public juce::Component
{
public:

	KeyButtons(const std::vector<juce::String>& keyNameWithOctave, const int& currentOctave)
	{

		std::array<const std::string_view, 102> notes = allNotes();

		m_keyButtons.reserve(keyNameWithOctave.size());

			for (size_t i = 0; i < keyNameWithOctave.size(); i++) {
				auto& button = m_keyButtons.emplace_back();
				button.setButtonText(keyNameWithOctave[i]);
				button.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
				button.setColour(juce::TextButton::buttonOnColourId, juce::Colours::lightgrey);
				button.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
				button.setNoteName(juce::String(notes[i].data(), notes[i].size()));
		}
	}

	void resized() override {
		auto area = getLocalBounds();
		const int keyWidth = area.getWidth() / 18;

		int keyButtonX = 75;
		int y = 100;
		int buttonWidth = 60;
		int buttonHeight = 120;

		/*	int x = 0;
			for (int i = m_currentOctave; i <= MAX_OCTAVE; i++) {
				m_keyButtons[i].setBounds(x, 0, keyWidth, area.getHeight() / 3);
				x += keyWidth;
			}*/

		int octaveStart = m_currentOctave * Constants::DISPLAYED_NOTES_SIZE;
		int previousOctaveStart = m_previousOctave * Constants::DISPLAYED_NOTES_SIZE;

		// accesses Past buttons and set visibility to false
		for (int i = previousOctaveStart; i < previousOctaveStart + Constants::DISPLAYED_NOTES_SIZE; ++i) {
			auto& keyButton = getKeyButton(i);
			keyButton.setVisible(false);
		}

		// sets bounds for current notes
		for (int i = octaveStart; i < octaveStart + Constants::DISPLAYED_NOTES_SIZE; ++i)
		{
			auto& keyButton = getKeyButton(i);
			if (i == octaveStart + 1 || i == octaveStart + 3 || i == octaveStart + 6 || i == octaveStart + 8 || i == octaveStart + 10 || 
				i == octaveStart + 13 || i == octaveStart + 15)
			{
				keyButton.setPosition(keyButtonX + 2);
				keyButton.setBounds(keyButtonX + 2, y - 10, buttonWidth - 5, buttonHeight - 25);
				keyButton.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
				keyButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
				keyButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
				keyButton.setColour(juce::ComboBox::outlineColourId, juce::Colours::white);
				addAndMakeVisible(keyButton);
			}
			else {
				keyButton.setPosition(keyButtonX);
				keyButton.setBounds(keyButtonX, y, buttonWidth, buttonHeight);
				addAndMakeVisible(keyButton);
			}

		}
	}

	void triggerResized(const int& currentOctave) {
		m_previousOctave = m_currentOctave;
		m_currentOctave = currentOctave;
		resized();
	}

	KeyButton& getKeyButton(int i) {
		return m_keyButtons.at(i);
	}



private:

	std::vector<KeyButton> m_keyButtons;
	int m_currentOctave = 3;
	int m_previousOctave;
};
