#include <map>
#include <juce_gui_basics/juce_gui_basics.h>
#include <algorithm>
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