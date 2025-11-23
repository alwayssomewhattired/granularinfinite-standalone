
#pragma once
#include <vector>
#include <string>

class AudioFileParse
{
public:
	AudioFileParse();
	~AudioFileParse();

	std::vector<double> readAudioFileAsMono(const std::string& fileName);
	void applyHanningWindow();

	const std::vector<double>& getAudioData() const;

private:
	std::vector<double> m_audioData;
};
