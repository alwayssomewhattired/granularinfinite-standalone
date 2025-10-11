
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include "PluginEditor.h"



class KeyButton : public juce::TextButton,
				  public juce::FileDragAndDropTarget
{
public:
	using FileDroppedCallback = std::function<void(const juce::String& fullPath,
		const juce::String& fileName)>;

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
		for (auto& f : files)
			if (f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".mp3"))
				return true;

		return false;
	}

	void filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/) override
	{
		if (files.isEmpty()) return;

		assignedFile = files[0]; // take the first file for now

		auto fileName = juce::File(assignedFile).getFileNameWithoutExtension();
		trimmedFileName = fileName;
		if (files.isEmpty()) return;
		if (onFileDropped) 
		{
			onFileDropped(assignedFile, fileName);
			// in here
			// use fileName to create + add button to map in buttonPalette.
			// make sure to attach lambda to onClick property of every button
			//audioEditor.waveformButtonHandler();
			//buttonPalette.addWaveformButton(fileName);
		}
	}


private:
	juce::String noteName;
	juce::String assignedFile;
	juce::String trimmedFileName;
	juce::String sampleName;

	// buttonPalette from editor
	//ButtonPalette& buttonPalette;
	//GranularinfiniteAudioProcessorEditor& audioEditor;

	FileDroppedCallback onFileDropped;

};
