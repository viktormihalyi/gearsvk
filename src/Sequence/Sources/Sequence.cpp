#include "Sequence.h"
#include "Response.h"
#include "Stimulus.h"
#include "SpatialFilter.h"
#include <algorithm>
#include <sstream>


Sequence::Sequence (std::string name)
    : name (name)
    , brief ("<no description>")
    , duration (0)
    , measurementStartOffset (0)
    , measurementEndOffset (0)
    , fieldWidth_um (2000)
    , fieldHeight_um (2000)
    , fieldLeft_px (0)
    , fieldBottom_px (0)
    , fieldWidth_px (1024)
    , fieldHeight_px (1024)
    , fftWidth_px (1024)
    , fftHeight_px (1024)
    , maxKernelHeight_um (0)
    , maxKernelWidth_um (0)
    , maxParticleGridWidth (0)
    , maxParticleGridHeight (0)
    , maxMemoryLength (0)
    , maxTemporalProcessingStateCount (0)
    , frameRateDivisor (1)
    , deviceFrameRate (59.94f)
    , tickInterval (0.00834167500834167500834167500834f)
    , hasFft (false)
    , hasSpatialDomainConvolution (false)
    , shortestStimulusDuration (1000000)
    , greyscale (false)
    , usesForwardRendering (false)
    , usesBusyWaitingThreadForSingals (false)
    , mono (true)
    , temp_r (nullptr)
    , usesDynamicToneMapping (false)
{
}


Sequence::Sequence ()
    : Sequence ("<no name>")
{
}


Sequence::~Sequence () = default;


void Sequence::addResponse (std::shared_ptr<Response> response)
{
    if (!temp_r) {
        response->setSequence (shared_from_this ());
        //response->joiner ();
        temp_r = response;

        temp_r->startingFrame = (duration + 1);
    } else {
        temp_r->duration    = duration - temp_r->startingFrame;
        responses[duration] = temp_r;
        temp_r              = nullptr;
    }
}

void Sequence::addStimulus (std::shared_ptr<Stimulus> stimulus)
{
    stimulus->setSequence (shared_from_this ());
    
    OnStimulusAdded (stimulus);

    mono = mono && stimulus->mono;

    shortestStimulusDuration = std::min (shortestStimulusDuration, stimulus->getDuration ());
    duration += stimulus->setStartingFrame (duration + 1);
    stimuli[duration] = stimulus;

    if (stimulus->fullScreenTemporalFiltering) {
        maxMemoryLength                 = std::max (maxMemoryLength, stimulus->temporalMemoryLength);
        maxTemporalProcessingStateCount = std::max (maxTemporalProcessingStateCount, stimulus->temporalProcessingStateCount);
    }
    if (stimulus->spatialFilter) {
        maxKernelWidth_um  = std::max (maxKernelWidth_um, stimulus->spatialFilter->width_um);
        maxKernelHeight_um = std::max (maxKernelHeight_um, stimulus->spatialFilter->height_um);
        hasFft |= stimulus->spatialFilter->useFft;
        hasSpatialDomainConvolution |= !stimulus->spatialFilter->useFft;
    }
    maxParticleGridWidth  = std::max (maxParticleGridWidth, stimulus->particleGridWidth);
    maxParticleGridHeight = std::max (maxParticleGridHeight, stimulus->particleGridHeight);
    if (stimulus->usesForwardRendering) {
        usesForwardRendering = true;
    }
    if (stimulus->doesDynamicToneMapping) {
        usesDynamicToneMapping = true;
    }
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

void Sequence::raiseAndClearSignal (std::string channel, uint32_t holdFor)
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
    return false; // TODO RNG
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


std::shared_ptr<const Response> Sequence::getResponseAtFrame (uint32_t iFrame) const
{
    auto i = responses.lower_bound (iFrame);
    if (i == responses.end ()) {
        return nullptr;
    }

    return i->second;
}

#include <sstream>

std::shared_ptr<const Stimulus> Sequence::getStimulusAtFrame (uint32_t iFrame)
{
    auto i = stimuli.lower_bound (iFrame);
    if (i == stimuli.end ()) {
        std::stringstream ss;
        ss << "There is no stimulus for frame" << iFrame << ". This erroneous access is likely due to a tone mapping file created for a version of the sequence with different timings. Please recalibrate the sequence.";
        return nullptr;
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