
#if JUCE_WINDOWS
#define NOMINMAX
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <pybind11/embed.h>
namespace py = pybind11;

inline void openConsole()
{
    static bool consoleOpened = false;
    if (consoleOpened)
        return;

    AllocConsole();

    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    std::ios::sync_with_stdio();

    consoleOpened = true;
}
#endif

//#pragma once
//#include <JuceHeader.h>
//#if JUCE_WINDOWS
//#define NOMINMAX
//#include <windows.h>
//#include <io.h>
//#include <fcntl.h>
//
//void openConsole()
//{
//	AllocConsole();
//
//	FILE* fp;
//	freopen_s(&fp, "CONOUT$", "w", stdout);
//	freopen_s(&fp, "CONOUT$", "w", stderr);
//	freopen_s(&fp, "CONIN", "r", stdin);
//
//	std::ios::sync_with_stdio();
//}
//
//#endif