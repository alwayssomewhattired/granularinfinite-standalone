
#include "PythonSoulseek.h"
#include <pybind11/embed.h>
namespace py = pybind11;

static bool pythonStarted = false;

juce::String runPythonFunction(const juce::String& songName)
{
	try {
		py::gil_scoped_acquire acquire;  // Ensure GIL is held here

		// Redirect Python stdout/stderr
		py::module sys = py::module::import("sys");

		auto slsk = py::module::import("slsk_python");
		auto result = slsk.attr("download_song")(songName.toStdString());
		return juce::String(result.cast<std::string>());
	}
	catch (const std::exception& e) {
		return "Python error: " + juce::String(e.what());
	}
}