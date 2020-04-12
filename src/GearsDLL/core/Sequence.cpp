#include "core/Sequence.h"
#include "PythonDict.h"
#include "core/Response.h"
#include "filter/SpatialFilter.h"
#include "stdafx.h"
#include <iostream>
#include <sstream>


Sequence::Sequence (std::string name)
    : name (name), brief ("<no description>"), duration (0), measurementStartOffset (0), measurementEndOffset (0), fieldWidth_um (2000), fieldHeight_um (2000), fieldLeft_px (0), fieldBottom_px (0), fieldWidth_px (1024), fieldHeight_px (1024), fftWidth_px (1024), fftHeight_px (1024), maxKernelHeight_um (0), maxKernelWidth_um (0), maxRandomGridWidth (0), maxRandomGridHeight (0), maxParticleGridWidth (0), maxParticleGridHeight (0), maxMemoryLength (0), maxTemporalProcessingStateCount (0), exportRandomsWithHashmark (true), exportRandomsChannelCount (1), exportRandomsAsReal (false), exportRandomsAsBinary (true), frameRateDivisor (1), deviceFrameRate (59.94f), tickInterval (0.00834167500834167500834167500834f), hasFft (false), hasSpatialDomainConvolution (false), shortestStimulusDuration (1000000), greyscale (false), usesForwardRendering (false), usesBusyWaitingThreadForSingals (false), mono (true), temp_r (nullptr), usesDynamicToneMapping (false)
{
}

Sequence::~Sequence ()
{
    stimuli.clear ();
    responses.clear ();
}

void Sequence::addResponse (Response::P response)
{
    if (!temp_r) {
        response->setSequence (getSharedPtr ());
        //response->joiner ();
        temp_r = response;

        temp_r->startingFrame = (duration + 1);
    } else {
        temp_r->duration    = duration - temp_r->startingFrame;
        responses[duration] = temp_r;
        temp_r              = nullptr;
    }
}

void Sequence::addStimulus (Stimulus::P stimulus)
{
    stimulus->setSequence (getSharedPtr ());
    stimulus->joiner ();
    mono                     = mono && stimulus->mono;
    shortestStimulusDuration = min (shortestStimulusDuration, stimulus->getDuration ());
    duration += stimulus->setStartingFrame (duration + 1);
    stimuli[duration] = stimulus;

    if (stimulus->fullScreenTemporalFiltering) {
        maxMemoryLength                 = max (maxMemoryLength, stimulus->temporalMemoryLength);
        maxTemporalProcessingStateCount = max (maxTemporalProcessingStateCount, stimulus->temporalProcessingStateCount);
    }
    if (stimulus->spatialFilter) {
        maxKernelWidth_um  = max (maxKernelWidth_um, stimulus->spatialFilter->width_um);
        maxKernelHeight_um = max (maxKernelHeight_um, stimulus->spatialFilter->height_um);
        hasFft |= stimulus->spatialFilter->useFft;
        hasSpatialDomainConvolution |= !stimulus->spatialFilter->useFft;
    }
    if (stimulus->randomSeed != 0) {
        maxRandomGridWidth  = max (maxRandomGridWidth, stimulus->randomGridWidth);
        maxRandomGridHeight = max (maxRandomGridHeight, stimulus->randomGridHeight);
    }
    maxParticleGridWidth  = max (maxParticleGridWidth, stimulus->particleGridWidth);
    maxParticleGridHeight = max (maxParticleGridHeight, stimulus->particleGridHeight);
    if (stimulus->usesForwardRendering) {
        usesForwardRendering = true;
    }
    if (stimulus->doesDynamicToneMapping) {
        usesDynamicToneMapping = true;
    }
}

pybind11::object Sequence::set (pybind11::object settings)
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
	using namespace boost::python;
	dict d = extract<dict>(settings);
	Gears::PythonDict pd(d);
	pd.process( [&](std::string key) {
		if(key == "geometry") {
			Gears::PythonDict geomSettings = pd.getSubDict(key);
			geomSettings.process( [&](std::string key) {
				if(		key == "fieldWidth_um")		fieldWidth_um	= geomSettings.getFloat(key);
				else if(key == "fieldHeight_um")	fieldHeight_um	= geomSettings.getFloat(key);
				else if(key == "fieldWidth_px")		fieldWidth_px	= geomSettings.getUint(key);
				else if(key == "fieldHeight_px")	fieldHeight_px	= geomSettings.getUint(key);
				else if(key == "fieldLeft_px")		fieldLeft_px	= geomSettings.getUint(key);
				else if(key == "fieldBottom_px")	fieldBottom_px	= geomSettings.getUint(key);
			});
		}
	});
#endif
    return settings;
}

bool Sequence::setMeasurementStart ()
{
    if (measurementStartOffset != 0)
        return false;
    measurementStartOffset = duration + 1;
    return true;
}

bool Sequence::setMeasurementEnd ()
{
    if (measurementEndOffset != 0)
        return false;
    measurementEndOffset = duration + 1;
    return true;
}

void Sequence::addChannel (std::string channelName, std::string portName, std::string bitName)
{
    channels[channelName].portName = portName;
#if 0
    if (bitName == "RTS") {
        channels[channelName].clearFunc = SETRTS;
        channels[channelName].raiseFunc = CLRRTS;
    }
    if (bitName == "BREAK") {
        channels[channelName].clearFunc = SETBREAK;
        channels[channelName].raiseFunc = CLRBREAK;
    }
    if (bitName == "DTR") {
        channels[channelName].clearFunc = SETDTR;
        channels[channelName].raiseFunc = CLRDTR;
    }
#elif __linux__
#endif
}

void Sequence::raiseSignal (std::string channel)
{
    SignalEvent e;
    e.clear   = false;
    e.channel = channel;
    signals.insert (std::pair<unsigned int, SignalEvent> (duration + 1, e));
}

void Sequence::clearSignal (std::string channel)
{
    SignalEvent e;
    e.clear   = true;
    e.channel = channel;
    signals.insert (std::pair<unsigned int, SignalEvent> (duration + 1, e));
}

void Sequence::raiseAndClearSignal (std::string channel, uint holdFor)
{
    SignalEvent e;
    e.clear   = false;
    e.channel = channel;
    signals.insert (std::pair<unsigned int, SignalEvent> (duration + 1, e));
    e.clear   = true;
    e.channel = channel;
    signals.insert (std::pair<unsigned int, SignalEvent> (duration + 1 + holdFor, e));
}

bool Sequence::usesRandoms ()
{
    return maxRandomGridWidth > 0 && maxRandomGridHeight > 0;
}

bool Sequence::usesParticles ()
{
    return maxParticleGridWidth > 0 && maxParticleGridHeight > 0;
}

float Sequence::getTimeForFrame (unsigned int frame)
{
    // Time since the start of stimulus
    // time is 1, it is the end of the stimulus
    return frame / deviceFrameRate * frameRateDivisor;
}

Sequence::P Sequence::setAgenda (pybind11::object agenda)
{
    using namespace pybind11;
    list                     l = agenda.cast<list> ();
    std::vector<Stimulus::P> stimsUnderConstruction;
    for (int i = 0; i < len (l); i++) {
        {
            extract<Response::P> s (l[i]);
            if (s.check ()) {
                addResponse (s ());
                continue;
            }
        }
        {
            extract<Stimulus::P> s (l[i]);
            if (s.check ()) {
                Stimulus::P sp = s ();
                stimsUnderConstruction.push_back (sp);
                addStimulus (sp);
                continue;
            }
        }
        {
            extract<RaiseSignal::P> s (l[i]);
            if (s.check ()) {
                RaiseSignal::P p = s ();
                raiseSignal (p->getChannel ());
                continue;
            }
        }
        {
            extract<ClearSignal::P> s (l[i]);
            if (s.check ()) {
                ClearSignal::P p = s ();
                clearSignal (p->getChannel ());
                continue;
            }
        }
        {
            extract<RaiseAndClearSignal::P> s (l[i]);
            if (s.check ()) {
                RaiseAndClearSignal::P p = s ();
                raiseAndClearSignal (p->getChannel (), p->getHoldFrameCount ());
                continue;
            }
        }
        {
            extract<StartMeasurement::P> s (l[i]);
            if (s.check ()) {
                StartMeasurement::P p = s ();
                raiseSignal (p->getExpSyncChannel ());
                if (!setMeasurementStart ()) {
                    std::stringstream ss;
                    ss << "Item #" << i + 1 << " on agenda is a StartMeasurement, but the measurement has already been started.";
                    THROW_LOC ();
                    //PyErr_SetString (PyExc_TypeError, ss.str ().c_str ());
                    //boost::python::throw_error_already_set ();
                }
                continue;
            }
        }
        {
            extract<EndMeasurement::P> s (l[i]);
            if (s.check ()) {
                EndMeasurement::P p = s ();
                clearSignal (p->getExpSyncChannel ());
                raiseAndClearSignal (p->getMeasurementStopChannel (), p->getHoldStopFrameCount ());
                if (!setMeasurementEnd ()) {
                    std::stringstream ss;
                    ss << "Item #" << i + 1 << " on agenda is an EndMeasurement, but the measurement has already been ended.";
                    THROW_LOC ();
                    //PyErr_SetString (PyExc_TypeError, ss.str ().c_str ());
                    //boost::python::throw_error_already_set ();
                }
                continue;
            }
        }
        std::stringstream ss;
        ss << "Item #" << i + 1 << " on agenda is of unknown type.";
        //THROW_LOC ();
        //PyErr_SetString (PyExc_TypeError, ss.str ().c_str ());
        //boost::python::throw_error_already_set ();
    }
    {
        for (auto suc : stimsUnderConstruction)
            suc->onSequenceComplete ();
    }

    return getSharedPtr ();
}

pybind11::object Sequence::onReset (pybind11::object cb)
{
    resetCallback = cb;
    return resetCallback;
}

pybind11::object Sequence::setPythonObject (pybind11::object o)
{
    pythonObject = o;
    return pythonObject;
}

pybind11::object Sequence::getPythonObject ()
{
    return pythonObject;
}

Response::CP Sequence::getResponseAtFrame (uint iFrame) const
{
    auto i = responses.lower_bound (iFrame);
    if (i == responses.end ()) {
        return nullptr;
    }

    return i->second;
}

#include <sstream>

Stimulus::CP Sequence::getStimulusAtFrame (uint iFrame)
{
    auto i = stimuli.lower_bound (iFrame);
    if (i == stimuli.end ()) {
        std::stringstream ss;
        ss << "There is no stimulus for frame" << iFrame << ". This erroneous access is likely due to a tone mapping file created for a version of the sequence with different timings. Please recalibrate the sequence.";
        throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
        //PyErr_SetString (PyExc_TypeError, ss.str ().c_str ());
        //boost::python::throw_error_already_set ();
    }

    return i->second;
}

bool Sequence::getUsesForwardRendering ()
{
    return usesForwardRendering;
}

void Sequence::setBusyWaitingTickInterval (float tickInterval)
{
    usesBusyWaitingThreadForSingals = true;
    this->tickInterval              = tickInterval;
}

bool Sequence::getUsesDynamicToneMapping ()
{
    return usesDynamicToneMapping;
}