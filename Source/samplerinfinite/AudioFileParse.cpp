/*
#define _USE_MATH_DEFINES
#pragma once
#include "AudioFileParse.h"
#include <sndfile.h>
#include <iostream>
#include <cmath>

AudioFileParse::AudioFileParse(){}

AudioFileParse::~AudioFileParse(){}

const std::vector<double>& AudioFileParse::getAudioData() const
{
	return m_audioData;
}

std::vector<double> AudioFileParse::readAudioFileAsMono(const std::string& fileName)
{
	SF_INFO sfInfo;
	SNDFILE* inFile = sf_open(fileName.c_str(), SFM_READ, &sfInfo);

	if (!inFile)
	{
		std::cout << "Error opening file: " << fileName << "\n";
		return {};
	}

	size_t numFrames = sfInfo.frames;
	int numChannels = sfInfo.channels;
	std::vector<double> rawData(numFrames * numChannels);

	if (sf_read_double(inFile, rawData.data(), rawData.size()) <= 0)
	{
		std::cout << "Error reading audio data from: " << fileName << "\n";
		sf_close(inFile);
		return {};
	}

	sf_close(inFile);

	// channel logic
	if (numChannels == 1)
	{
		return rawData;
	}
	else if (numChannels == 2)
	{
		m_audioData.resize(numFrames);
		for (size_t i = 0; i < numFrames; ++i)
		{
			m_audioData[i] = (rawData[2 * i] + rawData[2 * i + 1]) / 2.0;
		}
		return m_audioData;
	}
	else
	{
		std::cout << "unsupported numer of channels: " << numChannels << "\n";
	}
}

void AudioFileParse::applyHanningWindow()
{
	int n = static_cast<int>(m_audioData.size());
	for (int i = 0; i < n; i++)
	{
		double hannValue = 0.5 * (1 - cos(2 * M_PI * i / (n - 1)));
		m_audioData[i] *= hannValue;
	}
}
*/