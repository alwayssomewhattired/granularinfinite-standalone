#include <map>
#include <juce_gui_basics/juce_gui_basics.h>
#include <algorithm>
#include <cmath>
#pragma once

inline std::map<char, juce::String> CreateKeyToNote(int octave)
{
	octave = std::clamp(octave, 0, 8);
	using KP = char;
	using JS = juce::String;
	std::map<KP, JS> map;

	map[KP('a')] = "C" + JS(octave);
	map[KP('w')] = "C#" + JS(octave);
	map[KP('s')] = "D" + JS(octave);
	map[KP('e')] = "D#" + JS(octave);
	map[KP('d')] = "E" + JS(octave);
	map[KP('f')] = "F" + JS(octave);
	map[KP('t')] = "F#" + JS(octave);
	map[KP('g')] = "G" + JS(octave);
	map[KP('y')] = "G#" + JS(octave);
	map[KP('h')] = "A" + JS(octave);
	map[KP('u')] = "A#" + JS(octave);
	map[KP('j')] = "B" + JS(octave);
	map[KP('k')] = "C" + JS(octave + 1);
	map[KP('o')] = "C#" + JS(octave + 1);
	map[KP('l')] = "D" + JS(octave + 1);
	map[KP('p')] = "D#" + JS(octave + 1);
	map[KP(';')] = "E" + JS(octave + 1);
	map[KP('\'')] = "F" + JS(octave + 1);

	return map;
}

inline std::map<juce::String, int> CreateNoteToMidi = {
	{ "C0", 12 }, { "C#0", 13 }, { "D0", 14 }, { "D#0", 15 }, { "E0", 16 }, { "F0", 17 }, { "F#0", 18 }, { "G0", 19 }, { "G#0", 20 }, { "A0", 21 }, { "A#0", 22 }, { "B0", 23 },

	{ "C1", 24 }, { "C#1", 25 }, { "D1", 26 }, { "D#1", 27 }, { "E1", 28 }, { "F1", 29 }, { "F#1", 30 }, { "G1", 31 }, { "G#1", 32 }, { "A1", 33 }, { "A#1", 34 }, { "B1", 35 },

	{ "C2", 36 }, { "C#2", 37 }, { "D2", 38 }, { "D#2", 39 }, { "E2", 40 }, { "F2", 41 }, { "F#2", 42 }, { "G2", 43 }, { "G#2", 44 }, { "A2", 45 }, { "A#2", 46 }, { "B2", 47 },

	{ "C3", 48 }, { "C#3", 49 }, { "D3", 50 }, { "D#3", 51 }, { "E3", 52 }, { "F3", 53 }, { "F#3", 54 }, { "G3", 55 }, { "G#3", 56 }, { "A3", 57 }, { "A#3", 58 }, { "B3", 59 },

	{ "C4", 60 }, { "C#4", 61 }, { "D4", 62 }, { "D#4", 63 }, { "E4", 64 }, { "F4", 65 }, { "F#4", 66 }, { "G4", 67 }, { "G#4", 68 }, { "A4", 69 }, { "A#4", 70 }, { "B4", 71 },

	{ "C5", 72 }, { "C#5", 73 }, { "D5", 74 }, { "D#5", 75 }, { "E5", 76 }, { "F5", 77 }, { "F#5", 78 }, { "G5", 79 }, { "G#5", 80 }, { "A5", 81 }, { "A#5", 82 }, { "B5", 83 },

	{ "C6", 84 }, { "C#6", 85 }, { "D6", 86 }, { "D#6", 87 }, { "E6", 88 }, { "F6", 89 }, { "F#6", 90 }, { "G6", 91 }, { "G#6", 92 }, { "A6", 93 }, { "A#6", 94 }, { "B6", 95 },

	{ "C7", 96 }, { "C#7", 97 }, { "D7", 98 }, { "D#7", 99 }, { "E7", 100 }, { "F7", 101 }, { "F#7", 102 }, { "G7", 103 }, { "G#7", 104 }, { "A7", 105 }, { "A#7", 106 }, { "B7", 107 },

	{ "C8", 108 }, { "C#8", 109 }, { "D8", 110 }, { "D#8", 111 }, { "E8", 112 }, { "F8", 113 }
};

// make a map of the actual freq values
// this function is so overused
inline std::map<std::string, double> createNoteToFreq()
{
	std::map<std::string, double> noteToFreq;
	const std::string noteNames[] = { "C", "C#", "D", "D#", "E", "F",
									  "F#", "G", "G#", "A", "A#", "B" };

	const double A4_FREQ = 440.0;
	const int A4_INDEX = 9;
	const int A4_OCTAVE = 4;

	for (int octave = 0; octave <= 8; ++octave)
	{
		for (int i = 0; i < 12; ++i)
		{
			int semitoneOffset = (octave - A4_OCTAVE) * 12 + (i - A4_INDEX);
			double freq = A4_FREQ * std::pow(2.0, semitoneOffset / 12.0);

			std::string noteName = noteNames[i] + std::to_string(octave);
			noteToFreq[noteName] = freq;
		}
	}

	return noteToFreq;
}