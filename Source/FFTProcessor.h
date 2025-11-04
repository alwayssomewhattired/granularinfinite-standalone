#pragma once

#include <vector>
#include <unordered_map>
#include <fftw3.h>

class FFTProcessor {
public:
	FFTProcessor(int chunkSize, int sampleRate);

	~FFTProcessor();

	/**
	 * @param resetSamples clears the m_sampleStorage 
	*/
	void compute(const std::vector<double>& audioData, std::vector<double> targetFrequency, const int productLength, 
		bool resetSamples = true);

	const std::vector<std::vector<double>>& getMagnitudes() const;

	std::unordered_map<int, std::vector<double>>& getSampleStorage();

private:
	/**
	* @param ratio ***NEEDS CONTROL***
	*/
	bool isProminentPeak(const std::vector<double>& vec, double targetFrequency, double ratio = 0.8);
	void storeChunkIfProminent(const std::vector<double>& samples, int counter, double magnitude, double targetFrequency, const int productLength);

	int m_chunkSize;
	int m_sampleRate;
	int m_fftSize;

	// points to fftw_alloc_real
	double* m_realInput;
	fftw_complex* m_complexOutput;
	fftw_plan m_plan;

	std::vector<std::vector<double>> m_magnitudeChunks;
	std::unordered_map<int, std::vector<double>> m_sampleStorage;
};