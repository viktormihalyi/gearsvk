
// from Gears
#include "GearsAPIv2.hpp"
#include "PySequence/core/PyPass.h"
#include "PySequence/core/PyResponse.h"
#include "PySequence/core/PyStimulus.h"
#include "PySequence/core/PySequence.h"
#include "Sequence/SpatialFilter.h"
#include "PySequence/event/events.h"

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

#include "RenderGraph/GraphRenderer.hpp"

// Python requires an exported function called init<module-name> in every
// extension module. This is where we build the module contents.


std::shared_ptr<PySequence> sequence;

/*
std::shared_ptr<SequenceRenderer> sequenceRenderer = nullptr;
std::shared_ptr<ShaderManager>    shaderManager    = nullptr;
std::shared_ptr<TextureManager>   textureManager   = nullptr;
std::shared_ptr<KernelManager>    kernelManager    = nullptr;
*/
//std::shared_ptr<StimulusWindow>   stimulusWindow   = nullptr;


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


std::shared_ptr<PySequence> createPySequence (std::string name)
{
    ::sequence = std::make_shared<PySequence> (name);
    return ::sequence;
}


std::shared_ptr<PySequence> setSequence (std::shared_ptr<PySequence> sequence, const std::string& name)
{
    ::sequence = sequence;

    //textureManager->clear ();
    //shaderManager->clear ();
    //kernelManager->clear ();
    //sequenceRenderer->apply (::sequence, shaderManager, textureManager, kernelManager);

    Gears::SetRenderGraphFromSequence (sequence, name);

    return sequence;
}


std::shared_ptr<PySequence> getSequence ()
{
    return ::sequence;
}


void pickStimulus (double x, double y)
{
    //if (sequenceRenderer == nullptr)
    //    return;
    //sequenceRenderer->pickStimulus (x, y);
}



std::shared_ptr<const Stimulus> getSelectedStimulus ()
{
    return sequence->getStimuli ().rbegin ()->second;
    // return sequenceRenderer->getSelectedStimulus ();
}



void reset ()
{
    //sequenceRenderer->reset ();
}


void skip (int skipCount)
{
    //sequenceRenderer->skip (skipCount);
}


/*
std::shared_ptr<const Stimulus> getCurrentStimulus ()
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
    uint32_t screenw = GetSystemMetrics (SM_CXSCREEN);
    uint32_t screenh = GetSystemMetrics (SM_CYSCREEN);
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
std::shared_ptr<Ticker> startTicker ()
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


void enableCalibration (uint32_t startingFrame, uint32_t duration, float histogramMin, float histogramMax)
{
    //sequenceRenderer->enableCalibration (startingFrame, duration, histogramMin, histogramMax);
}


void setSequenceTimelineZoom (uint32_t nFrames)
{
    //sequenceRenderer->setSequenceTimelineZoom (nFrames);
}


void setSequenceTimelineStart (uint32_t iStartFrame)
{
    //sequenceRenderer->setSequenceTimelineStart (iStartFrame);
}


void setStimulusTimelineZoom (uint32_t nFrames)
{
    //sequenceRenderer->setStimulusTimelineZoom (nFrames);
}


void setStimulusTimelineStart (uint32_t iStartFrame)
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
pybind11::object renderSample (uint32_t iFrame, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
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
    //register_ptr_to_python<Gears::Event::std::shared_ptr<Base>>();

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

    
    class_<SpatialFilter, std::shared_ptr<SpatialFilter>> (m, "SpatialFilter")
        .def (init<> (&std::make_shared<SpatialFilter>))
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

    class_<Pass, std::shared_ptr<Pass>> passBase (m, "PassBase");
    passBase.def (init<> (&std::make_shared<Pass>))
        //.def( "set", &PyStimulus::set)
        .def ("getDuration", &Pass::getDuration)
        .def ("getDuration_s", &Pass::getDuration_s)
        .def ("getStartingFrame", &Pass::getStartingFrame)
        .def ("getStimulus", &Pass::getStimulus)
        .def ("getSequence", &Pass::getSequence)
        .def_readwrite ("name", &Pass::name, "Stimulus name.")
        .def_readwrite ("stimulusGeneratorShaderSource", &Pass::stimulusGeneratorShaderSource, "Sets GLSL source for stimulus generator shader.")
        .def_readwrite ("timelineVertexShaderSource", &Pass::timelineVertexShaderSource, "Sets GLSL source for timeline shader.")
        .def_readwrite ("timelineFragmentShaderSource", &Pass::timelineFragmentShaderSource, "Sets GLSL source for timeline shader.")
        .def_readwrite ("transparent", &Pass::transparent, "If True, the alpha mask is used for blending. Otherwise intensities are just added.")
        .def ("setShaderImage", &Pass::setShaderImage, arg ("name"), arg ("file"))
        .def ("setShaderFunction", &Pass::setShaderFunction, arg ("name"), arg ("src"))
        .def ("setGeomShaderFunction", &Pass::setGeomShaderFunction, arg ("name"), arg ("src"))
        .def ("setShaderVector", &Pass::setShaderVector, arg ("name"), arg ("x") = 0, arg ("y") = 0)
        .def ("setShaderColor", &Pass::setShaderColor, arg ("name"), arg ("all") = -2, arg ("red") = 0, arg ("green") = 0, arg ("blue") = 0)
        .def ("setShaderVariable", &Pass::setShaderVariable, arg ("name"), arg ("value"))
        .def ("setVideo", &Pass::setVideo)
        .def ("registerTemporalFunction", &Pass::registerTemporalFunction)
        .def ("setMotionTransformFunction", &Pass::setMotionTransformFunction)
        .def ("enableColorMode", &Pass::enableColorMode)
        .def_property ("duration", &Pass::getDuration, &Pass::setDuration, "Pass duration in frames.");

    class_<PyPass, std::shared_ptr<PyPass>> (m, "Pass", passBase)
        .def (init<> (&std::make_shared<PyPass>))
        .def ("setJoiner", &PyPass::setJoiner)
        .def ("setPythonObject", &PyPass::setPythonObject)
        .def ("getPythonObject", &PyPass::getPythonObject)
        .def ("setPolygonMask", &PyPass::setPolygonMask);

    class_<Response, std::shared_ptr<Response>> responseBase (m, "ResponseBase");
    responseBase.def (init<> (&std::make_shared<Response>))
        .def_readwrite ("question", &Response::question)
        .def_readwrite ("loop", &Response::loop)
        .def ("addButton", &Response::addButton)
        .def ("getSequence", &Response::getSequence);

    class_<PyResponse, std::shared_ptr<PyResponse>> (m, "Response", responseBase)
        .def (init<> (&std::make_shared<PyResponse>))
        .def ("setPythonObject", &PyResponse::setPythonObject)
        .def ("setJoiner", &PyResponse::setJoiner)
        .def ("registerCallback", &PyResponse::registerCallback);

    class_<Stimulus, std::shared_ptr<Stimulus>> stimulusBase (m, "StimulusBase");
    stimulusBase.def (init<> (&std::make_shared<Stimulus>))
        //.def( "set", &Stimulus::set)
        .def ("addPass", &Stimulus::addPass)
        .def ("getDuration", &Stimulus::getDuration)
        .def ("getDuration_s", &Stimulus::getDuration_s)
        .def ("getStartingFrame", &Stimulus::getStartingFrame)
        .def ("getSequence", &Stimulus::getSequence)
        .def_readwrite ("name", &Stimulus::name, "Stimulus name.")
        //.def_readwrite(	"stimulusGeneratorShaderSource"	, &Stimulus::stimulusGeneratorShaderSource	, "Sets GLSL source for stimulus generator shader.")
        //.def_readwrite(	"timelineVertexShaderSource"	, &Stimulus::timelineVertexShaderSource		, "Sets GLSL source for timeline shader.")
        //.def_readwrite(	"timelineFragmentShaderSource"	, &Stimulus::timelineFragmentShaderSource	, "Sets GLSL source for timeline shader.")
        .def_readwrite ("randomGeneratorShaderSource", &Stimulus::randomGeneratorShaderSource, "Sets GLSL source for random generator shader.")
        .def_readwrite ("randomGridHeight", &Stimulus::randomGridHeight, "Chessboard height [fields].")
        .def_readwrite ("randomGridWidth", &Stimulus::randomGridWidth, "Chessboard width [fields].")
        .def_readwrite ("randomSeed", &Stimulus::randomSeed, "Random seed.")
        .def_readwrite ("freezeRandomsAfterFrame", &Stimulus::randomSeed, "Stop generating new randoms after this many frames of the stimulus.")
        .def_readwrite ("particleShaderSource", &Stimulus::particleShaderSource, "Sets GLSL source for particle system shader.")
        .def_readwrite ("particleGridHeight", &Stimulus::particleGridHeight, "Particle grid height.")
        .def_readwrite ("particleGridWidth", &Stimulus::particleGridWidth, "Particle grid  width.")
        .def_readwrite ("requiresClearing", &Stimulus::requiresClearing, "If True, the screen is cleared before rendering the stimulus. Useful for multi-pass or instanced rendering. The default is False.")
        .def_readonly ("measuredToneRangeVar", &Stimulus::measuredVariance, "Measured variance.")
        .def_readonly ("measuredToneRangeMean", &Stimulus::measuredMean, "Measured mean.")
        .def_readonly ("measuredToneRangeMin", &Stimulus::measuredToneRangeMin, "Measured min.")
        .def_readonly ("measuredToneRangeMax", &Stimulus::measuredToneRangeMax, "Measured max.")
        .def_readwrite ("toneRangeVar", &Stimulus::toneRangeVar, "Scale applied in sigmoidal tone mapping.")
        .def_readwrite ("toneRangeMean", &Stimulus::toneRangeMean, "Offset applied in sigmoidal tone mapping.")
        .def_readwrite ("toneRangeMin", &Stimulus::toneRangeMin, "Linear tone mapping tone mapped to zero.")
        .def_readwrite ("toneRangeMax", &Stimulus::toneRangeMax, "Linear tone mapping tone mapped to one.")
        .def ("setToneMappingErf", &Stimulus::setToneMappingErf)
        .def ("setToneMappingLinear", &Stimulus::setToneMappingLinear)
        .def ("setToneMappingEqualized", &Stimulus::setToneMappingEqualized)
        //.def( "setShaderImage", &Stimulus::setShaderImage, ( arg("name"), arg("file")) )
        //.def( "setShaderFunction", &Stimulus::setShaderFunction, ( arg("name"), arg("src")) )
        //.def( "setShaderVector"		, &Stimulus::setShaderVector, ( arg("name"), arg("x")=0, arg("y")=0 ) )
        //.def( "setShaderColor", &Stimulus::setShaderColor, ( arg("name"), arg("all")=-2, arg("red")=0, arg("green")=0, arg("blue")=0) )
        //.def( "setShaderVariable", &Stimulus::setShaderVariable, ( arg("name"), arg("value")) )
        .def ("setClearColor", &Stimulus::setClearColor, arg ("all") = -2, arg ("red") = 0, arg ("green") = 0, arg ("blue") = 0)
        .def ("setSpatialFilter", &Stimulus::setSpatialFilter)
        .def ("getSpatialFilter", &Stimulus::getSpatialFilter)
        .def ("getSpatialPlotMin", &Stimulus::getSpatialPlotMin)
        .def ("getSpatialPlotMax", &Stimulus::getSpatialPlotMax)
        .def ("getSpatialPlotWidth", &Stimulus::getSpatialPlotWidth)
        .def ("getSpatialPlotHeight", &Stimulus::getSpatialPlotHeight)
        .def ("raiseSignalOnTick", &Stimulus::raiseSignalOnTick)
        .def ("clearSignalOnTick", &Stimulus::clearSignalOnTick)
        .def ("overrideTickSignals", &Stimulus::overrideTickSignals)
        .def ("hasSpatialFiltering", &Stimulus::hasSpatialFiltering)
        .def ("hasTemporalFiltering", &Stimulus::hasTemporalFiltering)
        .def ("doesErfToneMapping", &Stimulus::doesErfToneMapping)
        .def ("usesChannel", &Stimulus::usesChannel)
        .def ("getChannelCount", &Stimulus::getChannelCount)
        .def ("enableColorMode", &Stimulus::enableColorMode)
        .def_property ("duration", &Stimulus::getDuration, &Stimulus::setDuration, "Stimulus duration in frames.")
        .def ("addTag", &Stimulus::addTag);
        //.def("onStart", &Stimulus::onStart )
        //.def("onFrame", &Stimulus::onFrame )
        //.def("onFinish", &Stimulus::onFinish )

    class_<PyStimulus, std::shared_ptr<PyStimulus>> (m, "Stimulus", stimulusBase)
        .def (init<> (&std::make_shared<PyStimulus>))
        .def ("registerCallback", &PyStimulus::registerCallback)
        //.def("executeCallbacks", &Stimulus::executeCallbacks )
        .def ("setJoiner", &PyStimulus::setJoiner)
        .def ("setForwardRenderingCallback", &PyStimulus::setForwardRenderingCallback)
        .def ("setGamma", &PyStimulus::setGamma, arg ("gammaList"), arg ("invert") = false)
        .def ("setPythonObject", &PyStimulus::setPythonObject)
        .def ("getPythonObject", &PyStimulus::getPythonObject)
        .def ("setTemporalWeights", &PyStimulus::setTemporalWeights)
        .def ("setTemporalWeightingFunction", &PyStimulus::setTemporalWeightingFunction)
        .def ("setLtiMatrix", &PyStimulus::setLtiMatrix)
        .def ("setLtiImpulseResponse", &PyStimulus::setLtiImpulseResponse)
        .def ("setMeasuredDynamics", &PyStimulus::setMeasuredDynamicsFromPython)
        .def ("getMeasuredHistogramAsPythonList", &PyStimulus::getMeasuredHistogramAsPythonList);

    class_<Sequence::RaiseSignal, std::shared_ptr<Sequence::RaiseSignal>> (m, "RaiseSignal")
        .def (init<> (&std::make_shared<Sequence::RaiseSignal, std::string>));

    class_<Sequence::ClearSignal, std::shared_ptr<Sequence::ClearSignal>> (m, "ClearSignal")
        .def (init<> (&std::make_shared<Sequence::ClearSignal, std::string>));

    class_<Sequence::RaiseAndClearSignal, std::shared_ptr<Sequence::RaiseAndClearSignal>> (m, "RaiseAndClearSignal")
        .def (init<> (&std::make_shared<Sequence::RaiseAndClearSignal, std::string, uint32_t>));

    class_<Sequence::StartMeasurement, std::shared_ptr<Sequence::StartMeasurement>> (m, "StartMeasurement")
        .def (init<> (&std::make_shared<Sequence::StartMeasurement, std::string>), arg ("sequenceSyncSignal") = "Exp sync");

    class_<Sequence::EndMeasurement, std::shared_ptr<Sequence::EndMeasurement>> (m, "EndMeasurement")
        .def (init<> (&std::make_shared<Sequence::EndMeasurement, std::string, std::string, uint32_t>), arg ("sequenceSyncSignal") = "Exp sync", arg ("sequenceStopSignal") = "Msr stop", arg ("holdFrameCount") = 1);

    class_<Sequence, std::shared_ptr<Sequence>> sequenceBase (m, "SequenceBase");
    sequenceBase.def (init<> (&std::make_shared<Sequence, std::string>))
        .def ("getDuration", &Sequence::getDuration)
        .def ("addStimulus", &Sequence::addStimulus)
        .def ("getShortestStimulusDuration", &Sequence::getShortestStimulusDuration)
        .def ("addChannel", &Sequence::addChannel)
        .def ("getChannels", &Sequence::getChannels)
        .def ("getChannelCount", &Sequence::getChannelCount)
        .def ("raiseSignal", &Sequence::raiseSignal)
        .def ("clearSignal", &Sequence::clearSignal)
        .def ("raiseAndClearSignal", &Sequence::raiseAndClearSignal)
        .def ("getFrameInterval_s", &Sequence::getFrameInterval_s)
        .def ("usesRandoms", &Sequence::usesRandoms)
        .def ("getStimulusAtFrame", &Sequence::getStimulusAtFrame)
        .def ("getStimuli", &Sequence::getStimuli)
        .def ("setBusyWaitingTickInterval", &Sequence::setBusyWaitingTickInterval)
        .def ("getUsesBusyWaitingThreadForSingals", &Sequence::getUsesBusyWaitingThreadForSingals)
        .def ("getSpatialFilteredFieldWidth_um", &Sequence::getSpatialFilteredFieldWidth_um)
        .def ("getSpatialFilteredFieldHeight_um", &Sequence::getSpatialFilteredFieldHeight_um)
        .def_readwrite ("name", &Sequence::name, "Sequence name.")
        .def_readwrite ("useHighFreqRender", &Sequence::useHighFreqRender, "Use high frequence device.")
        .def_readwrite ("useOpenCL", &Sequence::useOpenCL, "Use OpenCL for FFT.")
        .def_readwrite ("field_width_um", &Sequence::fieldWidth_um, "The horizontal extent of the light pattern appearing on the retina [um].")
        .def_readwrite ("field_height_um", &Sequence::fieldHeight_um, "The vertical extent of the light pattern appearing on the retina [um].")
        .def_readwrite ("field_width_px", &Sequence::fieldWidth_px, "The size of the light pattern in display device pixels.")
        .def_readwrite ("field_height_px", &Sequence::fieldHeight_px, "The size of the light pattern in display device pixels.")
        .def_readwrite ("field_left_px", &Sequence::fieldLeft_px, "The horizontal position of the light pattern in pixels.")
        .def_readwrite ("field_bottom_px", &Sequence::fieldBottom_px, "The vertical position of the light pattern in pixels.")
        .def_readwrite ("fft_width_px", &Sequence::fftWidth_px, "The horizontal resolution used for frequency domain filtering [pixels].")
        .def_readwrite ("fft_height_px", &Sequence::fftHeight_px, "The vertical resolution used for frequency domain filtering [pixels].")
        .def_readwrite ("monitorIndex", &Sequence::monitorIndex, "Index of the display device to be used for stimulus projection.")
        .def_readwrite ("deviceFrameRate", &Sequence::deviceFrameRate, "VSYNC frequency of projector device. [1/s].")
        .def_readwrite ("frameRateDivisor", &Sequence::frameRateDivisor, "VSYNC cycles per frame rendered. [1].")
        .def_readwrite ("exportRandomsWithHashmarkComments", &Sequence::exportRandomsWithHashmark, "True if comments (denoted with #) should be included in the random numbers file.")
        .def_readwrite ("exportRandomsChannelCount", &Sequence::exportRandomsChannelCount, "Number of randoms to export for a grid cell [1-4].")
        .def_readwrite ("exportRandomsAsReal", &Sequence::exportRandomsAsReal, "True if randoms should be exported as floating point numbers.")
        .def_readwrite ("exportRandomsAsBinary", &Sequence::exportRandomsAsBinary, "True if randoms should be exported as fair 0/1 values.")
        .def_readwrite ("greyscale", &Sequence::greyscale, "Setting this to true allows faster filtering, but no colors.");
    
    class_<PySequence, std::shared_ptr<PySequence>> (m, "Sequence", sequenceBase)
        .def (init<> (&std::make_shared<PySequence, std::string>))
        .def ("set", &PySequence::set)
        .def ("setAgenda", &PySequence::setAgenda)
        .def ("onReset", &PySequence::onReset)
        .def ("setPythonObject", &PySequence::setPythonObject)
        .def ("getPythonObject", &PySequence::getPythonObject);

    //class_<Ticker, std::shared_ptr<Ticker>> (m, "Ticker")
    //    .def (init<> (&Ticker::create<std::shared_ptr<SequenceRenderer>>))
    //    .def ("stop", &Ticker::stop)
    //    .def ("onBufferSwap", &Ticker::onBufferSwap);

    m.def ("setSequence", setSequence /*, return_value_policy<reference_existing_object>()*/);
    m.def ("getSequence", getSequence /*, return_value_policy<reference_existing_object>()*/);
    m.def ("pickStimulus", pickStimulus);
    m.def ("getSelectedStimulus", getSelectedStimulus);
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

    m.def ("SetCurrentSurface", Gears::SetCurrentSurface);
    m.def ("RenderFrame", Gears::RenderFrame);
    m.def ("GetGLSLResourcesForRandoms", Gears::GetGLSLResourcesForRandoms);

#if 0
    class_<std::shared_ptr<p2t>oly2TriWrapper> (m, "CDT",
                                  "This is poly2tri CDT class python version"
                                  "Poly2Tri Copyright (c) 2009-2010, Poly2Tri Contributors"
                                  "http://code.google.com/p/poly2tri/")
        .def (init<list> (), arg ("polyline"))
        .def ("triangulate", &std::shared_ptr<p2t>oly2TriWrapper::Triangulate)
        .def ("get_triangles", &std::shared_ptr<p2t>oly2TriWrapper::GetTriangles);
#endif
}