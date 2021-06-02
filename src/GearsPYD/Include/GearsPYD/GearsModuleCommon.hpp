
// from Gears
#include "GearsAPIv2.hpp"
#include "core/Pass.h"
#include "core/PythonDict.h"
#include "core/Response.h"
#include "core/Stimulus.h"
#include "core/Sequence.h"
#include "Sequence/SpatialFilter.h"
#include "event/events.h"
#include "stdafx.h"

// from std
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

// from pybind11
#include <pybind11/embed.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "GraphRenderer.hpp"

// Python requires an exported function called init<module-name> in every
// extension module. This is where we build the module contents.


PySequence::P         sequence         = nullptr;

/*
SequenceRenderer::P sequenceRenderer = nullptr;
ShaderManager::P    shaderManager    = nullptr;
TextureManager::P   textureManager   = nullptr;
KernelManager::P    kernelManager    = nullptr;
*/
//StimulusWindow::P   stimulusWindow   = nullptr;


std::string createStimulusWindow ()
{
    //sequenceRenderer = SequenceRenderer::create ();

    //StimulusWindow::registerClass ();
    //stimulusWindow = StimulusWindow::create ();
    //stimulusWindow->createWindow (false, 256, 256);
    //
    ////stimulusWindow->setSequenceRenderer (sequenceRenderer);

    //shaderManager  = ShaderManager::create ();
    //textureManager = TextureManager::create ();
    //kernelManager  = KernelManager::create (sequenceRenderer, shaderManager);
    // OPENCLFFT::staticInit ();
    return "";
    //return stimulusWindow->getSpecs ();
}


void onHideStimulusWindow (pybind11::object onHideCallback)
{
#if 0
    if (stimulusWindow) {
        stimulusWindow->onHide (onHideCallback);
    }
#endif
}


void showText ()
{
    if (::sequence == nullptr)
        return;
    //sequenceRenderer->showText ();
}


void setText (std::string tag, std::string text)
{
    if (::sequence == nullptr)
        return;
    //sequenceRenderer->setText (tag, text);
}


void drawSequenceTimeline (int x, int y, int w, int h)
{
    if (::sequence == nullptr)
        return;
    //glViewport (x, y, w, h);
    //sequenceRenderer->renderTimeline ();
}


void drawStimulusTimeline (int x, int y, int w, int h)
{
    if (::sequence == nullptr)
        return;
    //glViewport (x, y, w, h);
    //sequenceRenderer->renderSelectedStimulusTimeline ();
}


void drawSpatialKernel (float min, float max, float width, float height)
{
    if (::sequence == nullptr)
        return;
    //sequenceRenderer->renderSelectedStimulusSpatialKernel (min, max, width, height);
}


void drawTemporalKernel ()
{
    if (::sequence == nullptr)
        return;
    //sequenceRenderer->renderSelectedStimulusTemporalKernel ();
}


void toggleChannelsOrPreview ()
{
    if (::sequence == nullptr)
        return;
    //sequenceRenderer->toggleChannelsOrPreview ();
}


void drawSpatialProfile (float min, float max, float width, float height)
{
    if (::sequence == nullptr)
        return;
    //sequenceRenderer->renderSelectedStimulusSpatialProfile (min, max, width, height);
}


void updateSpatialKernel ()
{
    if (::sequence == nullptr)
        return;
    //sequenceRenderer->updateSpatialKernel (kernelManager);
}


double getTime ()
{
    if (::sequence == nullptr)
        return 0.0;
    return 0.0;
    //return sequenceRenderer->getTime ();
}


PySequence::P createPySequence (std::string name)
{
    ::sequence = PySequence::create (name);
    return ::sequence;
}


PySequence::P setSequence (PySequence::P sequence)
{
    ::sequence = sequence;

    //textureManager->clear ();
    //shaderManager->clear ();
    //kernelManager->clear ();
    //sequenceRenderer->apply (::sequence, shaderManager, textureManager, kernelManager);

    Gears::SetRenderGraphFromSequence (sequence);

    return sequence;
}


PySequence::P getSequence ()
{
    return ::sequence;
}


void pickStimulus (double x, double y)
{
    //if (sequenceRenderer == nullptr)
    //    return;
    //sequenceRenderer->pickStimulus (x, y);
}


/*
Stimulus::CP getSelectedStimulus ()
{
    return nullptr;
    // return sequenceRenderer->getSelectedStimulus ();
}
*/


void reset ()
{
    //sequenceRenderer->reset ();
}


void skip (int skipCount)
{
    //sequenceRenderer->skip (skipCount);
}


/*
Stimulus::CP getCurrentStimulus ()
{
    return nullptr;
    //return sequenceRenderer->getCurrentStimulus ();
}
*/


void cleanup ()
{
    //sequenceRenderer->cleanup ();
    //OpenCLCore::Destroy ();
    //	textureManager->clear();
    //	shaderManager->clear();
    //	kernelManager->clear();
}


void setMousePointerLocation (float x, float y)
{
    throw std::runtime_error (Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString ());
#if 0
#ifdef _WIN32
    uint screenw = GetSystemMetrics (SM_CXSCREEN);
    uint screenh = GetSystemMetrics (SM_CYSCREEN);
    SetCursorPos ((int)(screenw * x), (int)(screenh * y));
#elif __linux__
    if (stimulusWindow)
        stimulusWindow->setCursorPos ();
#endif
#endif
}


void instantlyRaiseSignal (std::string c)
{
    //sequenceRenderer->raiseSignal (c);
}


void instantlyClearSignal (std::string c)
{
    //sequenceRenderer->clearSignal (c);
}


int getSkippedFrameCount ()
{
    return 0;
    //return sequenceRenderer->getSkippedFrameCount ();
}


std::string getSequenceTimingReport ()
{
    return "";
    //return sequenceRenderer->getSequenceTimingReport ();
}

/*
Ticker::P startTicker ()
{
    return nullptr;
    //return sequenceRenderer->startTicker ();
}
*/


void enableExport (std::string path)
{
    //sequenceRenderer->enableExport (path);
}


void enableVideoExport (const char* path, int fr, int w, int h)
{
    //sequenceRenderer->enableVideoExport (path, fr, w, h);
}


void enableCalibration (uint startingFrame, uint duration, float histogramMin, float histogramMax)
{
    //sequenceRenderer->enableCalibration (startingFrame, duration, histogramMin, histogramMax);
}


void setSequenceTimelineZoom (uint nFrames)
{
    //sequenceRenderer->setSequenceTimelineZoom (nFrames);
}


void setSequenceTimelineStart (uint iStartFrame)
{
    //sequenceRenderer->setSequenceTimelineStart (iStartFrame);
}


void setStimulusTimelineZoom (uint nFrames)
{
    //sequenceRenderer->setStimulusTimelineZoom (nFrames);
}


void setStimulusTimelineStart (uint iStartFrame)
{
    //sequenceRenderer->setStimulusTimelineStart (iStartFrame);
}


void pauseRender ()
{
    //sequenceRenderer->pause ();
}


void setResponded ()
{
    //sequenceRenderer->setResponded ();
}


/*
pybind11::object renderSample (uint iFrame, uint x, uint y, uint w, uint h)
{
    return sequenceRenderer->renderSample (iFrame, x, y, w, h);
}
*/


void makePath (std::string path)
{
    std::filesystem::path bpath (path);
    if (!std::filesystem::exists (bpath.parent_path ()))
        std::filesystem::create_directories (bpath.parent_path ());
}


std::string getSpecs ()
{
    return "[specs]";
}


void makeCurrent ()
{
    //    if (!stimulusWindow)
    //        return;
    //    stimulusWindow->makeCurrent ();
}


void shareCurrent ()
{
}


void run ()
{
}


int loadTexture (std::string filename)
{
    //if (textureManager)
    //    return textureManager->loadTexture (filename)->getTextureHandle ();

    return -1;
}

void bindTexture (std::string filename)
{
    //if (textureManager) {
    //    Texture2D* tex = textureManager->loadTexture (filename);
    //    //glBindTexture (GL_TEXTURE_2D, tex->getTextureHandle ());
    //}
}


void FillModule (pybind11::module_& m)
{
    using namespace pybind11;

    //class_<Gears::Event::Base>("BaseEvent", no_init)
    //	.def_readonly(	"message"	, &Gears::Event::Base::message	, "Windows message.")
    //	.def_readonly(	"wParam"	, &Gears::Event::Base::wParam	, "Windows message wParam.")
    //	.def_readonly(	"lParam"	, &Gears::Event::Base::lParam	, "Windows message lParam.")
    //	;
    //register_ptr_to_python<Gears::Event::Base::P>();

    class_<Gears::Event::MouseMove> (m, "MouseMoveEvent")
        .def ("globalX", &Gears::Event::MouseMove::globalX)
        .def ("globalY", &Gears::Event::MouseMove::globalY)
        .def ("globalPercentX", &Gears::Event::MouseMove::globalPercentX)
        .def ("globalPercentY", &Gears::Event::MouseMove::globalPercentY)
        .def_readonly_static ("typeId", &Gears::Event::MouseMove::typeId, "Message ID.");

    class_<Gears::Event::MousePressedLeft> (m, "MousePressedLeftEvent")
        .def ("globalX", &Gears::Event::MousePressedLeft::globalX)
        .def ("globalY", &Gears::Event::MousePressedLeft::globalY)
        .def_readonly_static ("typeId", &Gears::Event::MousePressedLeft::typeId, "Message ID.");

    class_<Gears::Event::MouseReleasedLeft> (m, "MouseReleasedLeftEvent")
        .def ("globalX", &Gears::Event::MouseReleasedLeft::globalX)
        .def ("globalY", &Gears::Event::MouseReleasedLeft::globalY)
        .def ("globalPercentX", &Gears::Event::MouseReleasedLeft::globalPercentX)
        .def ("globalPercentY", &Gears::Event::MouseReleasedLeft::globalPercentY)
        .def_readonly_static ("typeId", &Gears::Event::MouseReleasedLeft::typeId, "Message ID.");

    class_<Gears::Event::MousePressedMiddle> (m, "MousePressedMiddleEvent")
        .def ("globalX", &Gears::Event::MousePressedMiddle::globalX)
        .def ("globalY", &Gears::Event::MousePressedMiddle::globalY)
        .def_readonly_static ("typeId", &Gears::Event::MousePressedMiddle::typeId, "Message ID.");

    class_<Gears::Event::MouseReleasedMiddle> (m, "MouseReleasedMiddleEvent")
        .def ("globalX", &Gears::Event::MouseReleasedMiddle::globalX)
        .def ("globalY", &Gears::Event::MouseReleasedMiddle::globalY)
        .def_readonly_static ("typeId", &Gears::Event::MouseReleasedMiddle::typeId, "Message ID.");

    class_<Gears::Event::MousePressedRight> (m, "MousePressedRightEvent")
        .def ("globalX", &Gears::Event::MousePressedRight::globalX)
        .def ("globalY", &Gears::Event::MousePressedRight::globalY)
        .def_readonly_static ("typeId", &Gears::Event::MousePressedRight::typeId, "Message ID.");

    class_<Gears::Event::MouseReleasedRight> (m, "MouseReleasedRightEvent")
        .def ("globalX", &Gears::Event::MouseReleasedRight::globalX)
        .def ("globalY", &Gears::Event::MouseReleasedRight::globalY)
        .def_readonly_static ("typeId", &Gears::Event::MouseReleasedRight::typeId, "Message ID.");

    class_<Gears::Event::Wheel> (m, "WheelEvent")
        .def ("deltaX", &Gears::Event::Wheel::deltaX)
        .def ("deltaY", &Gears::Event::Wheel::deltaY)
        .def_readonly_static ("typeId", &Gears::Event::Wheel::typeId, "Message ID.");

    class_<Gears::Event::KeyPressed> (m, "KeyPressedEvent")
        .def ("text", &Gears::Event::KeyPressed::text)
        .def ("key", &Gears::Event::KeyPressed::key)
        .def_readonly_static ("typeId", &Gears::Event::KeyPressed::typeId, "Message ID.");

    class_<Gears::Event::KeyReleased> (m, "KeyReleasedEvent")
        .def ("text", &Gears::Event::KeyReleased::text)
        .def ("key", &Gears::Event::KeyReleased::key)
        .def_readonly_static ("typeId", &Gears::Event::KeyReleased::typeId, "Message ID.");

    class_<Gears::Event::Frame> (m, "FrameEvent")
        .def_readonly ("index", &Gears::Event::Frame::iFrame, "Frame index.")
        .def_readonly ("time", &Gears::Event::Frame::time, "Frame time.")
        .def_readonly_static ("typeId", &Gears::Event::Frame::typeId, "Message ID.");

    class_<Gears::Event::StimulusStart> (m, "StimulusStartEvent")
        .def_readonly_static ("typeId", &Gears::Event::StimulusStart::typeId, "Message ID.");

    class_<Gears::Event::StimulusEnd> (m, "StimulusEndEvent")
        .def_readonly_static ("typeId", &Gears::Event::StimulusEnd::typeId, "Message ID.");


    //    def("onDisplay", onDisplay);
    //    def("onDisplayHidden", onDisplayHidden);
    m.def ("drawSequenceTimeline", drawSequenceTimeline);
    m.def ("drawStimulusTimeline", drawStimulusTimeline);
    m.def ("drawTemporalKernel", drawTemporalKernel);
    m.def ("drawSpatialKernel", drawSpatialKernel);
    m.def ("drawSpatialProfile", drawSpatialProfile);

    
    class_<SpatialFilter, SpatialFilter::P> (m, "SpatialFilter")
        .def (init<> (&SpatialFilter::create<>))
        .def ("setShaderFunction", &SpatialFilter::setShaderFunction, arg ("name"), arg ("src"))
        .def ("setShaderColor", &SpatialFilter::setShaderColor, arg ("name"), arg ("all") = -2, arg ("red") = 0, arg ("green") = 0, arg ("blue") = 0)
        .def ("setShaderVector", &SpatialFilter::setShaderVector, arg ("name"), arg ("x") = 0, arg ("y") = 0)
        .def ("setShaderVariable", &SpatialFilter::setShaderVariable, arg ("name"), arg ("value"))
        .def ("setSpatialDomainShader", &SpatialFilter::setSpatialDomainShader, "Set spatial domain filtering shader. Default is convolution. ")
        .def ("makeUnique", &SpatialFilter::makeUnique, "Given a unique ID to the filter. The filter will not be shared with other stimuli using the same filter with the same settings. This is useful for interactive filters.")
        .def_readwrite ("minimum", &SpatialFilter::minimum, "The minimum plotted value of the kernel function.")
        .def_readwrite ("maximum", &SpatialFilter::maximum, "The minimum plotted value of the kernel function.")
        .def_readwrite ("width_um", &SpatialFilter::width_um, "The horizontal extent of the spatial filter kernel [um].")
        .def_readwrite ("height_um", &SpatialFilter::height_um, "The vertical extent of the spatial filter kernel [um].")
        .def_readwrite ("horizontalSampleCount", &SpatialFilter::horizontalSampleCount, "The number of samples used in spatial domain convolution.")
        .def_readwrite ("verticalSampleCount", &SpatialFilter::verticalSampleCount, "The number of samples used in spatial domain convolution.")
        .def_readwrite ("useFft", &SpatialFilter::useFft, "If True, convolution in computed in the frequency domain, if False, in the spatial domain. Use FFT for large kernels on powerful computers.")
        .def_readwrite ("kernelGivenInFrequencyDomain", &SpatialFilter::kernelGivenInFrequencyDomain, "If True, the kernel function gives the kernel directly in the frquency domain.")
        .def_readwrite ("showFft", &SpatialFilter::showFft, "If true, the result of frequency domain spatial processing is not trnasformed back to the spatial domain.")
        .def_readwrite ("stimulusGivenInFrequencyDomain", &SpatialFilter::stimulusGivenInFrequencyDomain, "If true, the stimulus is assumed to be already in frequency domain and not transformed before frequency domain processing.");
    

    //	class_<StimulusWindow>("StimulusWindow", no_init)
    //		.def( "__init__", make_constructor( &PyStimulus::create<>) )
    //		.def( "makeCurrent", &StimulusWindow::makeCurrent, "Set the OpenGL context of the stimulus window as the current OpenGL context. " )
    //		;

    class_<Sequence::Channel> (m, "Channel")
        .def_readonly ("portName", &Sequence::Channel::portName, "Name of the port the channel is associated to.")
        .def_readonly ("raiseFunc", &Sequence::Channel::raiseFunc, "ID of the signal the channel is associated to.");

    class_<PyPass, PyPass::P> (m, "Pass")
        .def (init<> (&PyPass::create<>))
        //.def( "set", &PyStimulus::set)
        .def ("getDuration", &PyPass::getDuration)
        .def ("getDuration_s", &PyPass::getDuration_s)
        .def ("getStartingFrame", &PyPass::getStartingFrame)
        .def ("getStimulus", &PyPass::getStimulus)
        .def ("getSequence", &PyPass::getSequence)
        .def_readwrite ("name", &PyPass::name, "PyStimulus name.")
        .def_readwrite ("stimulusGeneratorShaderSource", &PyPass::stimulusGeneratorShaderSource, "Sets GLSL source for stimulus generator shader.")
        .def_readwrite ("timelineVertexShaderSource", &PyPass::timelineVertexShaderSource, "Sets GLSL source for timeline shader.")
        .def_readwrite ("timelineFragmentShaderSource", &PyPass::timelineFragmentShaderSource, "Sets GLSL source for timeline shader.")
        .def_readwrite ("transparent", &PyPass::transparent, "If True, the alpha mask is used for blending. Otherwise intensities are just added.")
        .def ("setShaderImage", &PyPass::setShaderImage, arg ("name"), arg ("file"))
        .def ("setShaderFunction", &PyPass::setShaderFunction, arg ("name"), arg ("src"))
        .def ("setGeomShaderFunction", &PyPass::setGeomShaderFunction, arg ("name"), arg ("src"))
        .def ("setShaderVector", &PyPass::setShaderVector, arg ("name"), arg ("x") = 0, arg ("y") = 0)
        .def ("setShaderColor", &PyPass::setShaderColor, arg ("name"), arg ("all") = -2, arg ("red") = 0, arg ("green") = 0, arg ("blue") = 0)
        .def ("setShaderVariable", &PyPass::setShaderVariable, arg ("name"), arg ("value"))
        .def ("setJoiner", &PyPass::setJoiner)
        .def ("setVideo", &PyPass::setVideo)
        .def ("setPythonObject", &PyPass::setPythonObject)
        .def ("getPythonObject", &PyPass::getPythonObject)
        .def ("registerTemporalFunction", &PyPass::registerTemporalFunction)
        .def ("setPolygonMask", &PyPass::setPolygonMask)
        .def ("setMotionTransformFunction", &PyPass::setMotionTransformFunction)
        .def ("enableColorMode", &PyPass::enableColorMode)
        .def_property ("duration", &PyPass::getDuration, &PyPass::setDuration, "Pass duration in frames.");

    class_<PyResponse, PyResponse::P> (m, "Response")
        .def (init<> (&PyResponse::create<>))
        .def_readwrite ("question", &PyResponse::question)
        .def_readwrite ("loop", &PyResponse::loop)
        .def ("setPythonObject", &PyResponse::setPythonObject)
        .def ("setJoiner", &PyResponse::setJoiner)
        .def ("registerCallback", &PyResponse::registerCallback)
        .def ("addButton", &PyResponse::addButton)
        .def ("getSequence", &PyResponse::getSequence);

    class_<PyStimulus, PyStimulus::P> (m, "Stimulus")
        .def (init<> (&PyStimulus::create<>))
        //.def( "set", &PyStimulus::set)
        .def ("addPass", &PyStimulus::addPass)
        .def ("getDuration", &PyStimulus::getDuration)
        .def ("getDuration_s", &PyStimulus::getDuration_s)
        .def ("getStartingFrame", &PyStimulus::getStartingFrame)
        .def ("getSequence", &PyStimulus::getSequence)
        .def_readwrite ("name", &PyStimulus::name, "PyStimulus name.")
        //.def_readwrite(	"stimulusGeneratorShaderSource"	, &PyStimulus::stimulusGeneratorShaderSource	, "Sets GLSL source for stimulus generator shader.")
        //.def_readwrite(	"timelineVertexShaderSource"	, &PyStimulus::timelineVertexShaderSource		, "Sets GLSL source for timeline shader.")
        //.def_readwrite(	"timelineFragmentShaderSource"	, &PyStimulus::timelineFragmentShaderSource	, "Sets GLSL source for timeline shader.")
        .def_readwrite ("randomGeneratorShaderSource", &PyStimulus::randomGeneratorShaderSource, "Sets GLSL source for random generator shader.")
        .def_readwrite ("randomGridHeight", &PyStimulus::randomGridHeight, "Chessboard height [fields].")
        .def_readwrite ("randomGridWidth", &PyStimulus::randomGridWidth, "Chessboard width [fields].")
        .def_readwrite ("randomSeed", &PyStimulus::randomSeed, "Random seed.")
        .def_readwrite ("freezeRandomsAfterFrame", &PyStimulus::randomSeed, "Stop generating new randoms after this many frames of the stimulus.")
        .def_readwrite ("particleShaderSource", &PyStimulus::particleShaderSource, "Sets GLSL source for particle system shader.")
        .def_readwrite ("particleGridHeight", &PyStimulus::particleGridHeight, "Particle grid height.")
        .def_readwrite ("particleGridWidth", &PyStimulus::particleGridWidth, "Particle grid  width.")
        .def_readwrite ("requiresClearing", &PyStimulus::requiresClearing, "If True, the screen is cleared before rendering the stimulus. Useful for multi-pass or instanced rendering. The default is False.")
        .def_readonly ("measuredToneRangeVar", &PyStimulus::measuredVariance, "Measured variance.")
        .def_readonly ("measuredToneRangeMean", &PyStimulus::measuredMean, "Measured mean.")
        .def_readonly ("measuredToneRangeMin", &PyStimulus::measuredToneRangeMin, "Measured min.")
        .def_readonly ("measuredToneRangeMax", &PyStimulus::measuredToneRangeMax, "Measured max.")
        .def_readwrite ("toneRangeVar", &PyStimulus::toneRangeVar, "Scale applied in sigmoidal tone mapping.")
        .def_readwrite ("toneRangeMean", &PyStimulus::toneRangeMean, "Offset applied in sigmoidal tone mapping.")
        .def_readwrite ("toneRangeMin", &PyStimulus::toneRangeMin, "Linear tone mapping tone mapped to zero.")
        .def_readwrite ("toneRangeMax", &PyStimulus::toneRangeMax, "Linear tone mapping tone mapped to one.")
        .def ("setMeasuredDynamics", &PyStimulus::setMeasuredDynamicsFromPython)
        .def ("getMeasuredHistogramAsPythonList", &PyStimulus::getMeasuredHistogramAsPythonList)
        .def ("setToneMappingErf", &PyStimulus::setToneMappingErf)
        .def ("setToneMappingLinear", &PyStimulus::setToneMappingLinear)
        .def ("setToneMappingEqualized", &PyStimulus::setToneMappingEqualized)
        //.def( "setShaderImage", &PyStimulus::setShaderImage, ( arg("name"), arg("file")) )
        //.def( "setShaderFunction", &PyStimulus::setShaderFunction, ( arg("name"), arg("src")) )
        //.def( "setShaderVector"		, &PyStimulus::setShaderVector, ( arg("name"), arg("x")=0, arg("y")=0 ) )
        //.def( "setShaderColor", &PyStimulus::setShaderColor, ( arg("name"), arg("all")=-2, arg("red")=0, arg("green")=0, arg("blue")=0) )
        //.def( "setShaderVariable", &PyStimulus::setShaderVariable, ( arg("name"), arg("value")) )
        .def ("setClearColor", &PyStimulus::setClearColor, arg ("all") = -2, arg ("red") = 0, arg ("green") = 0, arg ("blue") = 0)
        .def ("setSpatialFilter", &PyStimulus::setSpatialFilter)
        .def ("getSpatialFilter", &PyStimulus::getSpatialFilter)
        .def ("getSpatialPlotMin", &PyStimulus::getSpatialPlotMin)
        .def ("getSpatialPlotMax", &PyStimulus::getSpatialPlotMax)
        .def ("getSpatialPlotWidth", &PyStimulus::getSpatialPlotWidth)
        .def ("getSpatialPlotHeight", &PyStimulus::getSpatialPlotHeight)
        .def ("raiseSignalOnTick", &PyStimulus::raiseSignalOnTick)
        .def ("clearSignalOnTick", &PyStimulus::clearSignalOnTick)
        .def ("overrideTickSignals", &PyStimulus::overrideTickSignals)
        .def ("setTemporalWeights", &PyStimulus::setTemporalWeights)
        .def ("setTemporalWeightingFunction", &PyStimulus::setTemporalWeightingFunction)
        .def ("setLtiMatrix", &PyStimulus::setLtiMatrix)
        .def ("setLtiImpulseResponse", &PyStimulus::setLtiImpulseResponse)
        .def ("hasSpatialFiltering", &PyStimulus::hasSpatialFiltering)
        .def ("hasTemporalFiltering", &PyStimulus::hasTemporalFiltering)
        .def ("doesErfToneMapping", &PyStimulus::doesErfToneMapping)
        //.def("onStart", &PyStimulus::onStart )
        //.def("onFrame", &PyStimulus::onFrame )
        //.def("onFinish", &PyStimulus::onFinish )
        .def ("registerCallback", &PyStimulus::registerCallback)
        //.def("executeCallbacks", &PyStimulus::executeCallbacks )
        .def ("setJoiner", &PyStimulus::setJoiner)
        .def ("setForwardRenderingCallback", &PyStimulus::setForwardRenderingCallback)
        .def ("setGamma", &PyStimulus::setGamma, arg ("gammaList"), arg ("invert") = false)
        .def ("setPythonObject", &PyStimulus::setPythonObject)
        .def ("getPythonObject", &PyStimulus::getPythonObject)
        .def ("usesChannel", &PyStimulus::usesChannel)
        .def ("getChannelCount", &PyStimulus::getChannelCount)
        .def ("enableColorMode", &PyStimulus::enableColorMode)
        .def_property ("duration", &PyStimulus::getDuration, &PyStimulus::setDuration, "PyStimulus duration in frames.")
        .def ("addTag", &PyStimulus::addTag);

    class_<Sequence::RaiseSignal, Sequence::RaiseSignal::P> (m, "RaiseSignal")
        .def (init<> (&Sequence::RaiseSignal::create<std::string>));

    class_<Sequence::ClearSignal, Sequence::ClearSignal::P> (m, "ClearSignal")
        .def (init<> (&Sequence::ClearSignal::create<std::string>));

    class_<Sequence::RaiseAndClearSignal, Sequence::RaiseAndClearSignal::P> (m, "RaiseAndClearSignal")
        .def (init<> (&Sequence::RaiseAndClearSignal::create<std::string, uint>));

    class_<Sequence::StartMeasurement, Sequence::StartMeasurement::P> (m, "StartMeasurement")
        .def (init<> (&Sequence::StartMeasurement::create<std::string>), arg ("sequenceSyncSignal") = "Exp sync");

    class_<Sequence::EndMeasurement, Sequence::EndMeasurement::P> (m, "EndMeasurement")
        .def (init<> (&Sequence::EndMeasurement::create<std::string, std::string, uint>), arg ("sequenceSyncSignal") = "Exp sync", arg ("sequenceStopSignal") = "Msr stop", arg ("holdFrameCount") = 1);


    class_<PySequence, PySequence::P> (m, "Sequence")
        .def (init<> (&PySequence::create<std::string>))
        .def ("set", &PySequence::set)
        .def ("getDuration", &PySequence::getDuration)
        .def ("addStimulus", &PySequence::addStimulus)
        .def ("getShortestStimulusDuration", &PySequence::getShortestStimulusDuration)
        .def ("setAgenda", &PySequence::setAgenda)
        .def ("addChannel", &PySequence::addChannel)
        .def ("getChannels", &PySequence::getChannels)
        .def ("getChannelCount", &PySequence::getChannelCount)
        .def ("raiseSignal", &PySequence::raiseSignal)
        .def ("clearSignal", &PySequence::clearSignal)
        .def ("raiseAndClearSignal", &PySequence::raiseAndClearSignal)
        .def ("getFrameInterval_s", &PySequence::getFrameInterval_s)
        .def ("usesRandoms", &PySequence::usesRandoms)
        .def ("onReset", &PySequence::onReset)
        .def ("setPythonObject", &PySequence::setPythonObject)
        .def ("getPythonObject", &PySequence::getPythonObject)
        .def ("getStimulusAtFrame", &PySequence::getStimulusAtFrame)
        .def ("getStimuli", &PySequence::getStimuli)
        .def ("setBusyWaitingTickInterval", &PySequence::setBusyWaitingTickInterval)
        .def ("getUsesBusyWaitingThreadForSingals", &PySequence::getUsesBusyWaitingThreadForSingals)
        .def ("getSpatialFilteredFieldWidth_um", &PySequence::getSpatialFilteredFieldWidth_um)
        .def ("getSpatialFilteredFieldHeight_um", &PySequence::getSpatialFilteredFieldHeight_um)
        .def_readwrite ("name", &PySequence::name, "PySequence name.")
        .def_readwrite ("useHighFreqRender", &PySequence::useHighFreqRender, "Use high frequence device.")
        .def_readwrite ("useOpenCL", &PySequence::useOpenCL, "Use OpenCL for FFT.")
        .def_readwrite ("field_width_um", &PySequence::fieldWidth_um, "The horizontal extent of the light pattern appearing on the retina [um].")
        .def_readwrite ("field_height_um", &PySequence::fieldHeight_um, "The vertical extent of the light pattern appearing on the retina [um].")
        .def_readwrite ("field_width_px", &PySequence::fieldWidth_px, "The size of the light pattern in display device pixels.")
        .def_readwrite ("field_height_px", &PySequence::fieldHeight_px, "The size of the light pattern in display device pixels.")
        .def_readwrite ("field_left_px", &PySequence::fieldLeft_px, "The horizontal position of the light pattern in pixels.")
        .def_readwrite ("field_bottom_px", &PySequence::fieldBottom_px, "The vertical position of the light pattern in pixels.")
        .def_readwrite ("fft_width_px", &PySequence::fftWidth_px, "The horizontal resolution used for frequency domain filtering [pixels].")
        .def_readwrite ("fft_height_px", &PySequence::fftHeight_px, "The vertical resolution used for frequency domain filtering [pixels].")
        .def_readwrite ("monitorIndex", &PySequence::monitorIndex, "Index of the display device to be used for stimulus projection.")
        .def_readwrite ("deviceFrameRate", &PySequence::deviceFrameRate, "VSYNC frequency of projector device. [1/s].")
        .def_readwrite ("frameRateDivisor", &PySequence::frameRateDivisor, "VSYNC cycles per frame rendered. [1].")
        .def_readwrite ("exportRandomsWithHashmarkComments", &PySequence::exportRandomsWithHashmark, "True if comments (denoted with #) should be included in the random numbers file.")
        .def_readwrite ("exportRandomsChannelCount", &PySequence::exportRandomsChannelCount, "Number of randoms to export for a grid cell [1-4].")
        .def_readwrite ("exportRandomsAsReal", &PySequence::exportRandomsAsReal, "True if randoms should be exported as floating point numbers.")
        .def_readwrite ("exportRandomsAsBinary", &PySequence::exportRandomsAsBinary, "True if randoms should be exported as fair 0/1 values.")
        .def_readwrite ("greyscale", &PySequence::greyscale, "Setting this to true allows faster filtering, but no colors.");

    //class_<Ticker, Ticker::P> (m, "Ticker")
    //    .def (init<> (&Ticker::create<SequenceRenderer::P>))
    //    .def ("stop", &Ticker::stop)
    //    .def ("onBufferSwap", &Ticker::onBufferSwap);

    m.def ("setSequence", setSequence /*, return_value_policy<reference_existing_object>()*/);
    m.def ("getSequence", getSequence /*, return_value_policy<reference_existing_object>()*/);
    m.def ("pickStimulus", pickStimulus);
    //m.def ("getSelectedStimulus", getSelectedStimulus);
    m.def ("reset", reset);
    m.def ("cleanup", cleanup);
    m.def ("enableExport", enableExport);
    m.def ("enableVideoExport", enableVideoExport);
    m.def ("enableCalibration", enableCalibration);
    //m.def ("startTicker", startTicker);
    m.def ("skip", skip);
    // m.def ("getCurrentStimulus", getCurrentStimulus);
    //	def("setSwapInterval", setSwapInterval );
    m.def ("makePath", makePath);
    m.def ("instantlyRaiseSignal", instantlyRaiseSignal);
    m.def ("instantlyClearSignal", instantlyClearSignal);
    m.def ("setSequenceTimelineZoom", setSequenceTimelineZoom);
    m.def ("setSequenceTimelineStart", setSequenceTimelineStart);
    m.def ("setStimulusTimelineZoom", setStimulusTimelineZoom);
    m.def ("setStimulusTimelineStart", setStimulusTimelineStart);
    m.def ("pause", pauseRender);
    m.def ("getSkippedFrameCount", getSkippedFrameCount);
    m.def ("getSequenceTimingReport", getSequenceTimingReport);
    m.def ("setText", setText);
    m.def ("showText", showText);
    m.def ("createStimulusWindow", createStimulusWindow);
    m.def ("getSpecs", getSpecs);
    m.def ("makeCurrent", makeCurrent);
    m.def ("shareCurrent", shareCurrent);
    m.def ("run", run);
    m.def ("onHideStimulusWindow", onHideStimulusWindow);
    m.def ("loadTexture", loadTexture);
    m.def ("bindTexture", bindTexture);
    m.def ("setMousePointerLocation", setMousePointerLocation);
    m.def ("updateSpatialKernel", updateSpatialKernel);
    m.def ("getTime", getTime);
    // def ("renderSample", renderSample);
    m.def ("setResponded", setResponded);
    m.def ("toggleChannelsOrPreview", toggleChannelsOrPreview);


    m.def ("InitializeEnvironment", Gears::InitializeEnvironment);
    m.def ("DestroyEnvironment", Gears::DestroyEnvironment);
    m.def ("StartRendering", Gears::StartRendering);
    m.def ("CreateSurface", Gears::CreateSurface);
    m.def ("DestroySurface", Gears::DestroySurface);
    m.def ("DestroySurface", Gears::SetRenderGraphFromSequence);

    m.def ("SetCurrentSurface", Gears::SetCurrentSurface);
    m.def ("RenderFrame", Gears::RenderFrame);
    m.def ("GetGLSLResourcesForRandoms", Gears::GetGLSLResourcesForRandoms);

#if 0
    class_<p2t::Poly2TriWrapper> (m, "CDT",
                                  "This is poly2tri CDT class python version"
                                  "Poly2Tri Copyright (c) 2009-2010, Poly2Tri Contributors"
                                  "http://code.google.com/p/poly2tri/")
        .def (init<list> (), arg ("polyline"))
        .def ("triangulate", &p2t::Poly2TriWrapper::Triangulate)
        .def ("get_triangles", &p2t::Poly2TriWrapper::GetTriangles);
#endif
}