#pragma once

#include <map>
#include <string>

#include "SequenceAPI.hpp"
#include <memory>

class Stimulus;
class Response;

//! A structure that contains all sequence parameters.
class SEQUENCE_API Sequence : public std::enable_shared_from_this<Sequence> {
public:
    using StimulusMap = std::map<unsigned int, std::shared_ptr<Stimulus const>>;
    using ResponseMap = std::map<unsigned int, std::shared_ptr<Response const>>;
    struct Channel {
        std::string  portName;
        unsigned int raiseFunc;
        unsigned int clearFunc;
    };
    using ChannelMap = std::map<std::string, Channel>;
    struct SignalEvent {
        bool        clear;
        std::string channel;
    };
    using SignalMap = std::multimap<unsigned int, SignalEvent>;

private:
    StimulusMap               stimuli; //<	Stimulus descriptors mapped to their starting frame index.
    ChannelMap                channels;
    SignalMap                 signals;
    ResponseMap               responses;
    std::shared_ptr<Response> temp_r;

    float maxKernelWidth_um;  //< The maximum horizontal extent of the spatial kernels used, measured on the retina [um].
    float maxKernelHeight_um; //< The maximum vertical extent of the spatial kernels used, measured on the retina [um].

    uint32_t maxMemoryLength;
    uint32_t maxTemporalProcessingStateCount;
    bool mono;

    bool usesDynamicToneMapping;
    bool usesForwardRendering;
    bool usesBusyWaitingThreadForSingals;

    unsigned int duration; //< Total sequence duration, including any (black) stimuli before and after measurement start and endpoint [frames]

    uint32_t shortestStimulusDuration;

    unsigned int measurementStartOffset; //< Measurement starts in this frame [frame]
    unsigned int measurementEndOffset;   //< Stop in this frame [frame]

protected:
    bool setMeasurementStart ();
    bool setMeasurementEnd ();

public:
    Sequence (std::string name);

    std::string name;  //< Unique name.
    std::string brief; //< A short discription of the sequence.

    bool greyscale;

    // fft switch
    bool useOpenCL         = false;
    bool useHighFreqRender = false;

    // random generation
    uint32_t maxRandomGridWidth;        //<	Number of cells per row in 2D random grid array for random number generation.
    uint32_t maxRandomGridHeight;       //<	Number of cells per column in of 2D random grid array for random number generation.
    bool exportRandomsWithHashmark; //< Use Python-style comments when exporting random numbers.
    uint32_t exportRandomsChannelCount; //<	How many randoms to export per cell of 2D random grid array.
    bool exportRandomsAsReal;       //<	If true, export random numbers as floating point numbers, mapped to [0, 1]. If false, and exportRandomsAsBinary is also false, export them as integers.
    bool exportRandomsAsBinary;     //< If true, export random numbers as 0 or 1. If false, and exportRandomsAsReal is also false, export them as integers.

    uint32_t maxParticleGridWidth;  //<	Number of cells per row in 2D random grid array for random number generation.
    uint32_t maxParticleGridHeight; //<	Number of cells per column in of 2D random grid array for random number generation.

    // measurement config (geometry & electronics)
    float        fieldWidth_um;  //< The size of the light pattern appearing on the retina [um].
    float        fieldHeight_um; //< The size of the light pattern appearing on the retina [um].
    unsigned int fieldLeft_px;   //< In-window (on-screen) pattern position and size.
    unsigned int fieldBottom_px; //< In-window (on-screen) pattern position and size.
    unsigned int fieldWidth_px;  //< In-window (on-screen) pattern position and size.
    unsigned int fieldHeight_px; //< In-window (on-screen) pattern position and size.
    unsigned int queueWidth_px;  //< In-window (on-screen) pattern position and size.
    unsigned int queueHeight_px; //< In-window (on-screen) pattern position and size.

    float        deviceFrameRate;
    unsigned int frameRateDivisor;
    unsigned int monitorIndex;
    float        tickInterval;

    // spatial filtering
    unsigned int fftWidth_px;  //<	Horizontal resolution of Fast Fourier Transform for spatial filtering.
    unsigned int fftHeight_px; //<	Vertical resolution of Fast Fourier Transform for spatial filtering.


    uint32_t               getDuration () const { return duration; }
    float              getTimeForFrame (unsigned int frame);
    const StimulusMap& getStimuli () const { return stimuli; }
    const ChannelMap&  getChannels () const { return channels; }
    uint32_t               getChannelCount () const { return channels.size (); }
    const SignalMap&   getSignals () const { return signals; }

    //! Destructor. Deletes stimuli.
    virtual ~Sequence ();

    void setupGeometry (
        float        fieldWidth_um,
        float        fieldHeight_um,
        unsigned int fieldLeft_px,
        unsigned int fieldBottom_px,
        unsigned int fieldWidth_px,
        unsigned int fieldHeight_px)
    {
        this->fieldWidth_um  = fieldWidth_um;
        this->fieldHeight_um = fieldHeight_um;
        this->fieldLeft_px   = fieldLeft_px;
        this->fieldBottom_px = fieldBottom_px;
        this->fieldWidth_px  = fieldWidth_px;
        this->fieldHeight_px = fieldHeight_px;
    }

    void addResponse (std::shared_ptr<Response> response);
    //! Adds stimulus to sequence and sets the starting frame of the stimulus.
    void addStimulus (std::shared_ptr<Stimulus> stimulus);

    void addChannel (std::string channelName, std::string portName, std::string bitName);

    void raiseSignal (std::string channel);
    void clearSignal (std::string channel);
    void raiseAndClearSignal (std::string channel, uint32_t holdFor);

    class RaiseSignal : public std::enable_shared_from_this<RaiseSignal> {
        std::string channel;

    public:
        RaiseSignal (std::string channel)
            : channel (channel) {}

        std::string getChannel () const { return channel; }
    };
    class ClearSignal : public std::enable_shared_from_this<ClearSignal> {
        std::string channel;

    public:
        ClearSignal (std::string channel)
            : channel (channel) {}

        std::string getChannel () const { return channel; }
    };
    class RaiseAndClearSignal : public std::enable_shared_from_this<RaiseAndClearSignal> {
        std::string channel;
        uint32_t        holdFor;

    public:
        RaiseAndClearSignal (std::string channel, uint32_t holdFor)
            : channel (channel), holdFor (holdFor) {}

        std::string getChannel () const { return channel; }
        uint32_t        getHoldFrameCount () const { return holdFor; }
    };
    class StartMeasurement : public std::enable_shared_from_this<StartMeasurement> {
        std::string expSyncChannel;

    public:
        StartMeasurement (std::string expSyncChannel)
            : expSyncChannel (expSyncChannel) {}
        std::string getExpSyncChannel () const { return expSyncChannel; }
    };
    class EndMeasurement : public std::enable_shared_from_this<EndMeasurement> {
        std::string expSyncChannel;
        std::string measurementStopChannel;
        uint32_t        holdStopFrameCount;

    public:
        EndMeasurement (std::string expSyncChannel, std::string measurementStopChannel, uint32_t holdStopFrameCount)
            : expSyncChannel (expSyncChannel), measurementStopChannel (measurementStopChannel), holdStopFrameCount (holdStopFrameCount) {}

        std::string getExpSyncChannel () const { return expSyncChannel; }
        std::string getMeasurementStopChannel () const { return measurementStopChannel; }
        uint32_t        getHoldStopFrameCount () { return holdStopFrameCount; }
    };

    uint32_t getMeasurementStart () const { return measurementStartOffset; }
    uint32_t getMeasurementEnd () const { return measurementEndOffset; }

    float getMaxKernelWidth_um () const { return maxKernelWidth_um; }
    float getMaxKernelHeight_um () const { return maxKernelHeight_um; }

    float getSpatialFilteredFieldWidth_um () const { return maxKernelWidth_um + fieldWidth_um; }
    float getSpatialFilteredFieldHeight_um () const { return maxKernelHeight_um + fieldHeight_um; }

    float getFrameInterval_s () const
    {
        return frameRateDivisor / deviceFrameRate;
    }

    bool usesRandoms ();
    bool usesParticles ();
    bool getUsesForwardRendering ();
    bool getUsesDynamicToneMapping ();

    bool hasFft;
    bool hasSpatialDomainConvolution;

    uint32_t getShortestStimulusDuration () { return shortestStimulusDuration; }

    std::shared_ptr<Stimulus const> getStimulusAtFrame (uint32_t iFrame);
    std::shared_ptr<Response const> getResponseAtFrame (uint32_t iFrame) const;

    uint32_t getMaxMemoryLength () const { return maxMemoryLength; }
    uint32_t getMaxTemporalProcessingStateCount () const { return maxTemporalProcessingStateCount; }

    void setBusyWaitingTickInterval (float tickInterval);
    bool getUsesBusyWaitingThreadForSingals () const { return usesBusyWaitingThreadForSingals; }

    bool isMonochrome () const { return mono; }

    uint32_t getMonitorIndex () const { return monitorIndex; }

public:
    virtual void OnStimulusAdded (std::shared_ptr<Stimulus> stimulus) {}
};
