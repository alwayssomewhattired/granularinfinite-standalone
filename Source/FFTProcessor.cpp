
#pragma once
#include "FFTProcessor.h"
#include <iostream>
#include <JuceHeader.h>

FFTProcessor::FFTProcessor(int chunkSize, int sampleRate)
	: m_chunkSize(chunkSize), m_sampleRate(sampleRate)
{
	m_fftSize = chunkSize / 2 + 1;
	m_realInput = fftw_alloc_real(chunkSize);
	m_complexOutput = fftw_alloc_complex(m_fftSize);
	m_plan = fftw_plan_dft_r2c_1d(chunkSize, m_realInput, m_complexOutput, FFTW_MEASURE);
}

FFTProcessor::~FFTProcessor()
{
	if (m_plan)
	{
		fftw_destroy_plan(m_plan);
	}
	if (m_realInput)
	{
		fftw_free(m_realInput);
	}
	if (m_complexOutput)
	{
		fftw_free(m_complexOutput);
	}
}

bool FFTProcessor::isProminentPeak(const std::vector<double>& vec, double targetFrequency, double ratio)
{
	if (targetFrequency == 0) return false;

	double maxVal = *std::max_element(vec.begin(), vec.end());

	return targetFrequency >= ratio * maxVal;
}

void FFTProcessor::compute(const std::vector<double>& audioData, std::vector<double> targetFrequency, const int productLength, bool resetSamples)
{
	m_magnitudeChunks.clear();

	if (resetSamples)
	{
		m_sampleStorage.clear();
	}

	if (audioData.size() > static_cast<size_t>(std::numeric_limits<int>::max()))
	{
		std::cout << "Source Audio file too large for type conversion" << "\n" "Recevied size: " << audioData.size() << "\n";
		return;
	}

	int maxChunkSize = static_cast<int>(audioData.size());
	int numChunks = (maxChunkSize + m_chunkSize - 1) / m_chunkSize;

	for (int chunk = 0; chunk < numChunks; ++chunk)
	{
		std::fill(m_realInput, m_realInput + m_chunkSize, 0);

		int start = chunk * m_chunkSize;
		int end = std::min(start + m_chunkSize, maxChunkSize);

		std::copy(audioData.begin() + start, audioData.begin() + end, m_realInput);

		// do we really need to execute the plan on every chunk????
		fftw_execute(m_plan);

		std::vector<double> magnitudes(m_fftSize);
		for (int i = 0; i < m_fftSize; ++i)
		{
			magnitudes[i] = std::sqrt(m_complexOutput[i][0] * m_complexOutput[i][0] + m_complexOutput[i][1] * m_complexOutput[i][1]);
		}

		m_magnitudeChunks.push_back(std::move(magnitudes));

		for (double freq : targetFrequency)
		{
			int controlNoteBin = static_cast<int>(freq * m_chunkSize / m_sampleRate);
			if (controlNoteBin >= 0 && controlNoteBin < m_fftSize)
			{
				double targetFrequencyMagnitude = m_magnitudeChunks.back()[controlNoteBin];
				if (isProminentPeak(m_magnitudeChunks.back(), targetFrequencyMagnitude))
				{
					storeChunkIfProminent(audioData, chunk, targetFrequencyMagnitude, freq, productLength);
				}
			}
		}
	}
}

void FFTProcessor::storeChunkIfProminent(const std::vector<double>& samples, int counter, double magnitude, double targetFrequency, const int productLength)
{
	int start = counter * m_chunkSize;
	int end = std::min(start + m_chunkSize, static_cast<int>(samples.size()));
	int targetFrequencyi = std::round(targetFrequency);
	for (int i = start; i < end; ++i)
	{
		double val = std::clamp(samples[i], -1.0, 1.0);
		m_sampleStorage[targetFrequencyi].push_back(val);
	}
}

const std::vector<std::vector<double>>& FFTProcessor::getMagnitudes() const
{
	return m_magnitudeChunks;
}

std::unordered_map<int, std::vector<double>>& FFTProcessor::getSampleStorage()
{
	return m_sampleStorage;
}