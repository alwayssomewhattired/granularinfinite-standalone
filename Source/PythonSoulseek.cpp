
#include "PythonSoulseek.h"
#include <pybind11/embed.h>
namespace py = pybind11;

static bool pythonStarted = false;

//void initialisePython()
//{
//	if (!pythonStarted)
//	{
//		std::cout << "python initializing\n";
//		py::initialize_interpreter();
//
//		py::module sys = py::module::import("sys");
//		sys.attr("path").attr("append")("C:/Users/zacha/Desktop/granularinfinite/Source");
//		pythonStarted = true;
//	}
//}

juce::String runPythonFunction(const juce::String& songName)
{
	try {
		py::gil_scoped_acquire acquire;  // Ensure GIL is held here

		// Redirect Python stdout/stderr
		py::module sys = py::module::import("sys");

		std::cout << "running python function\n";
		auto slsk = py::module::import("slsk_python");
		std::cout << "before py script\n";
		auto result = slsk.attr("download_song")(songName.toStdString());
		std::cout << "after py script\n";
		return juce::String(result.cast<std::string>());
	}
	catch (const std::exception& e) {
		return "Python error: " + juce::String(e.what());
	}
}