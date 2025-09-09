
#pragma once
#include <JuceHeader.h>
#if JUCE_WINDOWS
#include <windows.h>
#include <io.h>
#include <fcntl.h>

void openConsole()
{
	AllocConsole();

	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONIN", "r", stdin);

	std::ios::sync_with_stdio();
}

#endif