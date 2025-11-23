
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
	using FileDroppedCallback = std::function<void(const juce::String& fullPath,
		const juce::String& fileName, std::map<juce::String, juce::Array<juce::File>>& noteToFiles)>;

	KeyButton(const juce::String& keyName, const juce::String& noteName)
		: TextButton(keyName), noteName(keyName)
	{
		setRepaintsOnMouseActivity(true);
	}

	void setOnFileDropped(FileDroppedCallback cb) { onFileDropped = std::move(cb); }

	juce::String getNoteName() const { return noteName; }

	// the filename including directory
	juce::String getAssignedFile() const { return assignedFile; }

	// the trimmed filename
	juce::String getTrimmedFileName() const { return trimmedFileName; }


	bool isInterestedInFileDrag(const juce::StringArray& files) override
	{	
		// temporarily commented out 
		//for (auto& f : files)
		//	if (f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".mp3"))
				return true;

		//return false;
	}

	//juce::String fileFreq;

	void collectAudioFiles(const juce::File& file, juce::Array<juce::File>& results, juce::String fileFreq) {
		if (!file.exists()) return;

		juce::String currentFreq;

		if (file.isDirectory()) {
			juce::Array<juce::File> children;
			file.findChildFiles(children, juce::File::findFilesAndDirectories, false);
			if (CreateNoteToMidi.find(file.getFileName()) != CreateNoteToMidi.end())
				currentFreq = file.getFileName();

			for (auto& child : children) collectAudioFiles(child, results, fileFreq);
		}
		else {
			if (file.hasFileExtension("wav;mp3;aiff;flac;ogg"))
				m_noteToFiles[currentFreq] = file;
		}
	}

	void filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/) override
	{
		if (files.isEmpty()) return;

		juce::Array<juce::File> audioFiles;
		juce::String fileFreq;

		for (auto& path : files)
			collectAudioFiles(juce::File(path), audioFiles, fileFreq);

		for (auto& audioFile : audioFiles) {
			const juce::String path = audioFile.getFullPathName();
			auto trimmed = audioFile.getFileNameWithoutExtension();

			if (onFileDropped)
				onFileDropped(path, trimmed, m_noteToFiles);
		}
	}


private:
	juce::String noteName;
	juce::String assignedFile;
	juce::String trimmedFileName;
	juce::String sampleName;

	std::map<juce::String, juce::Array<juce::File>> m_noteToFiles;

	FileDroppedCallback onFileDropped;

};
