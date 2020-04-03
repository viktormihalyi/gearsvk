#include "Utils.hpp"

#include <iostream>
#include <string>

#include <pybind11/pybind11.h>


template<typename ReturnType, typename... ARGS>
ReturnType Swallow (ARGS...)
{
    return ReturnType ();
}

struct Empty {
    int   x;
    void* dummy;
    Empty () {}
    Empty (std::string) {}
    USING_PTR (Empty);
};


namespace py = pybind11;
using py::arg;

PYBIND11_MODULE (Gears, m)
{
    m.doc () = R"pbdoc(
        Pybind11 example plugin
        -----------------------
        .. currentmodule:: python_example
        .. autosummary::
           :toctree: _generate
           add
           subtract
    )pbdoc";

    m.def (
        "subtract",
        [] (int i, int j) { return i - j; }, R"pbdoc(
        Subtract two numbers
        Some other explanation about the subtract function.
    )pbdoc");

#ifdef VERSION_INFO
    m.attr ("__version__") = VERSION_INFO;
#else
    m.attr ("__version__") = "dev";
#endif


    py::class_<Empty, Empty::P> (m, "Empty")
        .def (py::init ());

    py::class_<Empty, Empty::P> (m, "ShaderManager")
        .def ("loadShaderFromFile", &Swallow<void>)
        .def (py::init ());


    m.def ("drawSequenceTimeline", &Swallow<void>);
    m.def ("drawStimulusTimeline", &Swallow<void>);
    m.def ("drawTemporalKernel", &Swallow<void>);
    m.def ("drawSpatialKernel", &Swallow<void>);
    m.def ("drawSpatialProfile", &Swallow<void>);

    py::class_<Empty, Empty::P> (m, "SpatialFilter")
        .def ("__init__", &Empty::CreateShared<>)
        .def ("setShaderFunction", &Swallow<void>)
        .def ("setShaderColor", &Swallow<void>)
        .def ("setShaderVector", &Swallow<void>)
        .def ("setShaderVariable", &Swallow<void>)
        .def ("setSpatialDomainShader", &Swallow<void>, "Set spatial domain filtering shader. Default is convolution. ")
        .def ("makeUnique", &Swallow<void>, "Given a unique ID to the filter. The filter will not be shared with other stimuli using the same filter with the same settings. This is useful for interactive filters.")
        .def_readwrite ("minimum", &Empty::x, "The minimum plotted value of the kernel function.")
        .def_readwrite ("maximum", &Empty::x, "The minimum plotted value of the kernel function.")
        .def_readwrite ("width_um", &Empty::x, "The horizontal extent of the spatial filter kernel [um].")
        .def_readwrite ("height_um", &Empty::x, "The vertical extent of the spatial filter kernel [um].")
        .def_readwrite ("horizontalSampleCount", &Empty::x, "The number of samples used in spatial domain convolution.")
        .def_readwrite ("verticalSampleCount", &Empty::x, "The number of samples used in spatial domain convolution.")
        .def_readwrite ("useFft", &Empty::x, "If True, convolution in computed in the frequency domain, if False, in the spatial domain. Use FFT for large kernels on powerful computers.")
        .def_readwrite ("kernelGivenInFrequencyDomain", &Empty::x, "If True, the kernel function gives the kernel directly in the frquency domain.")
        .def_readwrite ("showFft", &Empty::x, "If true, the result of frequency domain spatial processing is not trnasformed back to the spatial domain.")
        .def_readwrite ("stimulusGivenInFrequencyDomain", &Empty::x, "If true, the stimulus is assumed to be already in frequency domain and not transformed before frequency domain processing.");


    py::class_<Empty, Empty::P> (m, "Channel")
        .def_readonly ("portName", &Empty::x, "Name of the port the channel is associated to.")
        .def_readonly ("raiseFunc", &Empty::x, "ID of the signal the channel is associated to.");

    py::class_<Empty, Empty::P> (m, "Pass")
        .def ("__init__", &Empty::CreateShared<>)
        .def ("getDuration", &Swallow<void>)
        .def ("getDuration_s", &Swallow<void>)
        .def ("getStartingFrame", &Swallow<void>)
        .def ("getStimulus", &Swallow<void>)
        .def ("getSequence", &Swallow<void>)
        .def_readwrite ("name", &Empty::x, "Stimulus name.")
        .def_readwrite ("stimulusGeneratorShaderSource", &Empty::x, "Sets GLSL source for stimulus generator shader.")
        .def_readwrite ("timelineVertexShaderSource", &Empty::x, "Sets GLSL source for timeline shader.")
        .def_readwrite ("timelineFragmentShaderSource", &Empty::x, "Sets GLSL source for timeline shader.")
        .def_readwrite ("transparent", &Empty::x, "If True, the alpha mask is used for blending. Otherwise intensities are just added.")
        .def ("setShaderImage", &Swallow<void, std::string, std::string>)
        .def ("setShaderFunction", &Swallow<void, std::string, std::string>)
        .def ("setGeomShaderFunction", &Swallow<void, std::string, std::string>)
        .def ("setShaderVector", &Swallow<void, std::string, int, int>)
        .def ("setShaderColor", &Swallow<void, int, int, int, int, int>)
        .def ("setShaderVariable", &Swallow<void, std::string, int>)
        .def ("setJoiner", &Swallow<void>)
        .def ("setVideo", &Swallow<void>)
        .def ("setPythonObject", &Swallow<void>)
        .def ("getPythonObject", &Swallow<void>)
        .def ("registerTemporalFunction", &Swallow<void>)
        .def ("setPolygonMask", &Swallow<void>)
        .def ("setMotionTransformFunction", &Swallow<void>)
        .def ("enableColorMode", &Swallow<void>)
        .def_property ("duration", &Swallow<int>, &Swallow<void, int>, "Pass duration in frames.");

    py::class_<Empty, Empty::P> (m, "Response")
        .def ("__init__", &Empty::CreateShared<>)
        .def_readwrite ("question", &Empty::x)
        .def_readwrite ("loop", &Empty::x)
        .def ("setPythonObject", &Swallow<void>)
        .def ("setJoiner", &Swallow<void>)
        .def ("registerCallback", &Swallow<void>)
        .def ("addButton", &Swallow<void>)
        .def ("getSequence", &Swallow<void>);

    py::class_<Empty, Empty::P> (m, "Stimulus")
        .def ("__init__", &Empty::CreateShared<>)
        //.def( "set", &Stimulus::set)
        .def ("addPass", &Swallow<void>)
        .def ("getDuration", &Swallow<void>)
        .def ("getDuration_s", &Swallow<void>)
        .def ("getStartingFrame", &Swallow<void>)
        .def ("getSequence", &Swallow<void>)
        .def_readwrite ("name", &Empty::x, "Stimulus name.")
        //.def_readwrite(	"stimulusGeneratorShaderSource"	, &Stimulus::stimulusGeneratorShaderSource	, "Sets GLSL source for stimulus generator shader.")
        //.def_readwrite(	"timelineVertexShaderSource"	, &Stimulus::timelineVertexShaderSource		, "Sets GLSL source for timeline shader.")
        //.def_readwrite(	"timelineFragmentShaderSource"	, &Stimulus::timelineFragmentShaderSource	, "Sets GLSL source for timeline shader.")
        .def_readwrite ("randomGeneratorShaderSource", &Empty::x, "Sets GLSL source for random generator shader.")
        .def_readwrite ("randomGridHeight", &Empty::x, "Chessboard height [fields].")
        .def_readwrite ("randomGridWidth", &Empty::x, "Chessboard width [fields].")
        .def_readwrite ("randomSeed", &Empty::x, "Random seed.")
        .def_readwrite ("freezeRandomsAfterFrame", &Empty::x, "Stop generating new randoms after this many frames of the stimulus.")
        .def_readwrite ("particleShaderSource", &Empty::x, "Sets GLSL source for particle system shader.")
        .def_readwrite ("particleGridHeight", &Empty::x, "Particle grid height.")
        .def_readwrite ("particleGridWidth", &Empty::x, "Particle grid  width.")
        .def_readwrite ("requiresClearing", &Empty::x, "If True, the screen is cleared before rendering the stimulus. Useful for multi-pass or instanced rendering. The default is False.")
        .def_readonly ("measuredToneRangeVar", &Empty::x, "Measured variance.")
        .def_readonly ("measuredToneRangeMean", &Empty::x, "Measured mean.")
        .def_readonly ("measuredToneRangeMin", &Empty::x, "Measured min.")
        .def_readonly ("measuredToneRangeMax", &Empty::x, "Measured max.")
        .def_readwrite ("toneRangeVar", &Empty::x, "Scale applied in sigmoidal tone mapping.")
        .def_readwrite ("toneRangeMean", &Empty::x, "Offset applied in sigmoidal tone mapping.")
        .def_readwrite ("toneRangeMin", &Empty::x, "Linear tone mapping tone mapped to zero.")
        .def_readwrite ("toneRangeMax", &Empty::x, "Linear tone mapping tone mapped to one.")
        .def ("setMeasuredDynamics", &Swallow<void>)
        .def ("getMeasuredHistogramAsPythonList", &Swallow<void>)
        .def ("setToneMappingErf", &Swallow<void>)
        .def ("setToneMappingLinear", &Swallow<void>)
        .def ("setToneMappingEqualized", &Swallow<void>)
        //.def( "setShaderImage", &Stimulus::setShaderImage, ( arg("name"), arg("file")) )
        //.def( "setShaderFunction", &Stimulus::setShaderFunction, ( arg("name"), arg("src")) )
        //.def( "setShaderVector"		, &Stimulus::setShaderVector, ( arg("name"), arg("x")=0, arg("y")=0 ) )
        //.def( "setShaderColor", &Stimulus::setShaderColor, ( arg("name"), arg("all")=-2, arg("red")=0, arg("green")=0, arg("blue")=0) )
        //.def( "setShaderVariable", &Stimulus::setShaderVariable, ( arg("name"), arg("value")) )
        .def ("setClearColor", &Swallow<void, int, int, int, int>)
        .def ("setSpatialFilter", &Swallow<void>)
        .def ("getSpatialFilter", &Swallow<void>)
        .def ("getSpatialPlotMin", &Swallow<void>)
        .def ("getSpatialPlotMax", &Swallow<void>)
        .def ("getSpatialPlotWidth", &Swallow<void>)
        .def ("getSpatialPlotHeight", &Swallow<void>)
        .def ("raiseSignalOnTick", &Swallow<void>)
        .def ("clearSignalOnTick", &Swallow<void>)
        .def ("overrideTickSignals", &Swallow<void>)
        .def ("setTemporalWeights", &Swallow<void>)
        .def ("setTemporalWeightingFunction", &Swallow<void>)
        .def ("setLtiMatrix", &Swallow<void>)
        .def ("setLtiImpulseResponse", &Swallow<void>)
        .def ("hasSpatialFiltering", &Swallow<void>)
        .def ("hasTemporalFiltering", &Swallow<void>)
        .def ("doesErfToneMapping", &Swallow<void>)
        //.def("onStart", &Stimulus::onStart )
        //.def("onFrame", &Stimulus::onFrame )
        //.def("onFinish", &Stimulus::onFinish )
        .def ("registerCallback", &Swallow<void>)
        //.def("executeCallbacks", &Stimulus::executeCallbacks )
        .def ("setJoiner", &Swallow<void>)
        .def ("setForwardRenderingCallback", &Swallow<void>)
        .def ("setGamma", &Swallow<void, int, bool>)
        .def ("setPythonObject", &Swallow<void>)
        .def ("getPythonObject", &Swallow<void>)
        .def ("usesChannel", &Swallow<void>)
        .def ("getChannelCount", &Swallow<void>)
        .def ("enableColorMode", &Swallow<void>)
        .def_property ("duration", &Swallow<void>, &Swallow<void>, "Stimulus duration in frames.")
        .def ("addTag", &Swallow<void>);
}