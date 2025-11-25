
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <functional>
#include "PluginEditor.h"
#include "KeyToNote.h"



class KeyButton : public juce::TextButton,
				  public juce::FileDragAndDropTarget
{
public:
	using FileDroppedCallback = std::function<void(std::map<juce::String, juce::Array<juce::File>>& noteToFiles, const bool& isDir)>;

	KeyButton(const juce::String& keyName, const juce::String& noteName)
		: TextButton(keyName), noteName(keyName)
	{
		setRepaintsOnMouseActivity(true);
	}

	void setOnFileDropped(FileDroppedCallback cb) { 
		onFileDropped = std::move(cb); 
	}

	juce::String getNoteName() const { return noteName; }

	// the filename including directory
	juce::String getAssignedFile() const { return assignedFile; }

	// the trimmed filename
	juce::String getTrimmedFileName() const { return trimmedFileName; }

	void setTrimmedFileName(const juce::String& filename) { trimmedFileName = filename; }

	bool isInterestedInFileDrag(const juce::StringArray& files) override
	{	
		// temporarily commented out 
		//for (auto& f : files)
		//	if (f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".mp3"))
				return true;

		//return false;
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


private:
	juce::String noteName;
	juce::String assignedFile;
	// filename without the extension
	juce::String trimmedFileName;

	juce::String sampleName;
	// Key: frequency
	// Value: folder of audio files
	std::map<juce::String, juce::Array<juce::File>> m_noteToFiles;
	bool m_isDir;

	FileDroppedCallback onFileDropped;

};
