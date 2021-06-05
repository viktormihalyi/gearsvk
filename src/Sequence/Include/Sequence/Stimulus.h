#pragma once

#include "SequenceAPI.hpp"

#include <memory>

#include <algorithm>

#include <iomanip>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <glm/glm.hpp>

class Pass;
class Sequence;
class SpatialFilter;

//! A structure that contains all stimulus parameters.
class SEQUENCE_API Stimulus : public std::enable_shared_from_this<Stimulus> {
    unsigned int duration; //< Stimulus duration [frames]. The number of frames we need to render in stimulus
public:
    std::string name;  //< Unique name.
    std::string brief; //< A short discription of the stimulus.

    std::shared_ptr<Sequence> sequence; //< Part of this sequence.

    std::vector<std::shared_ptr<Pass>> passes;
    void                               addPass (std::shared_ptr<Pass> pass);

    unsigned int startingFrame; //< Stimulus starts at this time in sequence [frames].

    struct SignalEvent {
        bool        clear;
        std::string channel;
    };
    using SignalMap = std::multimap<unsigned int, SignalEvent>;
    SignalMap             tickSignals;
    std::set<std::string> stimulusChannels;

    bool      requiresClearing;
    glm::vec3 clearColor;

    bool usesForwardRendering;

    std::string spikeVertexShaderSource; //< Renders timeline representation.
    std::string spikeFragmentShaderSource;

    std::string temporalFilterPlotVertexShaderSource; //< Renders timeline representation.
    std::string temporalFilterPlotFragmentShaderSource;

    std::string linearDynamicToneShaderSource;
    std::string erfDynamicToneShaderSource;
    std::string equalizedDynamicToneShaderSource;
    std::string temporalFilterShaderSource;
    std::string temporalFilterFuncSource;
    float       temporalWeights[64];
    uint32_t        temporalMemoryLength;
    bool        mono;
    uint32_t        temporalProcessingStateCount;
    bool        fullScreenTemporalFiltering;
    float       temporalWeightMax;
    float       temporalWeightMin;

    glm::mat4 temporalProcessingStateTransitionMatrix[4];

    std::shared_ptr<SpatialFilter> spatialFilter;

    std::string randomGeneratorShaderSource; //< Prng.
    uint32_t        randomGridWidth;             //<	Number of cells per row in 2D random grid array for random number generation.
    uint32_t        randomGridHeight;            //<	Number of cells per column in of 2D random grid array for random number generation.
    uint32_t        randomSeed;                  //< Initial number for random number generation. The same seed always produces the same randoms.
    uint32_t        freezeRandomsAfterFrame;

    std::string particleShaderSource; //< Particle system.
    uint32_t        particleGridWidth;    //<	Number of cells per row in 2D grid array.
    uint32_t        particleGridHeight;   //<	Number of cells per column in of 2D grid array.

    // tone dynamics
    float gamma[101];        //< The 'gamma correction' curve of the display device, from 0 to 1 in 0.01 resolution
    int   gammaSamplesCount; //< The number of meaningful elements is the gamma array. Cannot be higher than 101.

    bool doesToneMappingInStimulusGenerator;
    enum class ToneMappingMode { LINEAR,
                                 ERF,
                                 EQUALIZED };
    ToneMappingMode toneMappingMode;
    bool            doesDynamicToneMapping;          //< Compute histogram in every frame.
    float           histogramMeasurementImpedance;   //< 0-per frame, 0.99-slow adaptation
    bool            computesFullAverageForHistogram; //< histogram is the avarage histogram of all frames measured. histogramMeasurementImpedance is ignored
    float           stretchFactor;                   //< Scale measured linear-min-max-from-mean or variace values by this.
    float           meanOffset;                      //< Offset applied to measured mean.

    float toneRangeMin; //< The output stimulus value mapped to 0 on the display.
    float toneRangeMax; //< The output stimulus value mapped to 1 on the display.
    float toneRangeMean;
    float toneRangeVar;

    std::vector<float> measuredHistogram;
    float              measuredToneRangeMin;
    float              measuredToneRangeMax;
    float              measuredMean;
    float              measuredVariance;

    std::set<std::string> interactives;

    void overrideTickSignals ();
    void raiseSignalOnTick (uint32_t iTick, std::string channel);
    void clearSignalOnTick (uint32_t iTick, std::string channel);

    void finishLtiSettings ();

    unsigned int roundForHighFrequency (unsigned int);

public:
    Stimulus ();
    virtual ~Stimulus ();

    void setSequence (std::shared_ptr<Sequence> sequence);

    void onSequenceComplete ();

    unsigned int setStartingFrame (unsigned int offset);

    void saveConfig (const std::string& expName);

    void setDuration (unsigned int duration);
    uint32_t getDuration () const;

    std::string getRandomGeneratorShaderSource () const;

    std::string getParticleShaderSource () const;

    std::string getTemporalFilterShaderSource () const;

    std::string getTemporalFilterPlotVertexShaderSource () const;

    std::string getTemporalFilterPlotFragmentShaderSource () const;

    void setSpatialFilter (std::shared_ptr<SpatialFilter> spatialFilter);

    std::shared_ptr<const SpatialFilter> getSpatialFilter () const;

    float getSpatialPlotMin ();
    float getSpatialPlotMax ();
    float getSpatialPlotWidth ();
    float getSpatialPlotHeight ();

    uint32_t getStartingFrame () const;

    bool hasSpatialFiltering () const;

    bool hasTemporalFiltering () const;

    std::shared_ptr<Sequence> getSequence ();

    float getDuration_s () const;

    bool usesChannel (std::string channel);
    uint32_t getChannelCount ();

    void setMeasuredDynamics (float  measuredToneRangeMin,
                              float  measuredToneRangeMax,
                              float  measuredMean,
                              float  measuredVariance,
                              float* histi,
                              uint32_t   histogramResolution) const;

    void setToneMappingLinear (float min, float max, bool dynamic) const;
    void setToneMappingErf (float mean, float var, bool dynamic) const;
    void setToneMappingEqualized (bool dynamic) const;

    const SignalMap& getSignals () const;

    void enableColorMode ();

    const std::vector<std::shared_ptr<Pass>>& getPasses () const;

    void setClearColor (float all, float r, float g, float b);

    void addTag (std::string tag);

    const std::set<std::string>& getTags () const;

    bool doesErfToneMapping () const;

    std::string getDynamicToneShaderSource () const;

    bool IsEquivalent (const Stimulus& other) const;

    virtual void OnPassAdded (std::shared_ptr<Pass> pass) {}
};
