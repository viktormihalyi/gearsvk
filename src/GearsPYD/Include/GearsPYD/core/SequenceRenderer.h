#pragma once

#include "OpenGLProxy.hpp"
#include <memory>

#include "core/PortHandler.h"
#include "core/Stimulus.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

class Sequence;
class FramebufferGL;
class StimulusRenderer;
class Nothing;
class Shader;
class Texture1D;
class Response;
class Texture2D;
class RandomSequenceBuffer;
class TextureQueue;
class TextureManager;
class KernelManager;
class ShaderManager;
class Ticker;


//! Represents the currently active sequence. Manages resources for GPU computation.
class SequenceRenderer {
    friend class StimulusRenderer;
    friend class PassRenderer;
    friend class SpatialFilterRenderer;

    //! Active sequence.
    std::shared_ptr<Sequence const> sequence;
    FramebufferGL*                  spatialDomainFilteringBuffers[2];

    bool         paused;
    unsigned int iFrame;

    std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::high_resolution_clock::duration> firstFrameTimePoint;
    std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::high_resolution_clock::duration> previousFrameTimePoint;

    int              cFrame;
    int              totalFramesSkipped;
    std::vector<int> skippedFrames;

    bool isDrawingPreview;
    int  vSyncPeriodsSinceLastFrame = -1;

    using StimulusRendererMap = std::map<unsigned int, std::shared_ptr<StimulusRenderer>>;
    StimulusRendererMap           stimulusRenderers;
    StimulusRendererMap::iterator selectedStimulusRenderer;

    using PortMap = std::map<std::string, PortHandler>;
    PortMap ports;

    Stimulus::SignalMap noTickSignal;

    //	Quad* fullscreenQuad;
    Nothing* nothing;

    Shader* textShader; //< renders text

    std::map<std::string, std::string> text;
    bool                               textVisible;
    bool                               responseVisible;
    std::shared_ptr<Response const>    currentResponse;

    //! Index of current slice in texture queue. Changes continuously during sequence.
    unsigned int currentSlice;

    std::unique_ptr<RandomSequenceBuffer> randomSequenceBuffers[5];
    std::ofstream                         randomExportStream;
    RandomSequenceBuffer*                 particleBuffers[2];
    TextureQueue*                         textureQueue;
    TextureQueue*                         currentTemporalProcessingState;
    TextureQueue*                         nextTemporalProcessingState;
#if 0
    FontManager fontManager;
#endif
    bool           exportingToVideo;
    uint32_t           videoExportImageWidth;
    uint32_t           videoExportImageHeight;
    FramebufferGL* videoExportImage;
    FramebufferGL* videoExportImageY;
    FramebufferGL* videoExportImageU;
    FramebufferGL* videoExportImageV;
#if 0
    AVFrame*        frame;
    AVCodecContext* c;
    FILE*           videoExportFile;
    AVPacket        pkt;
#endif
    int     got_output;
    Shader* rgbToY;
    Shader* rgbToU;
    Shader* rgbToV;
    Shader* showVideoFrame;

    bool  calibrating;
    uint32_t  calibrationFrameCount;
    float histogramMin;
    float histogramMax;
    float histogramScale;
    uint32_t  histogramResolution;
    uint32_t  calibrationImageWidth;
    uint32_t  calibrationImageHeight;
    uint32_t  calibrationHorizontalSampleCount;
    uint32_t  calibrationVerticalSampleCount;

    uint32_t screenWidth;
    uint32_t screenHeight;

    FramebufferGL* forwardRenderedImage;

    Shader*        histogramShader;
    Shader*        histogramHalferShader;
    Shader*        histogramClearShader;
    Shader*        histogramDisplayShader;
    FramebufferGL* calibrationImage;
    FramebufferGL* histogramBuffer;
    FramebufferGL* histogramBuffer2;
    FramebufferGL* histogramBuffer3;

    float measuredToneRangeMin;
    float measuredToneRangeMax;
    float measuredMean;
    float measuredVariance;

    void readCalibrationResults ();

    GLboolean _redMaskByIndex[3]   = { GL_TRUE, GL_FALSE, GL_FALSE };
    GLboolean _greenMaskByIndex[3] = { GL_FALSE, GL_TRUE, GL_FALSE };
    GLboolean _blueMaskByIndex[3]  = { GL_FALSE, GL_FALSE, GL_TRUE };


public:
    SequenceRenderer ();

    //! Creates GPU resources for the sequence, releasing earlier ones, if any.
    void apply (std::shared_ptr<Sequence> sequence, std::shared_ptr<ShaderManager> shaderManager, std::shared_ptr<TextureManager> textureManager, std::shared_ptr<KernelManager> kernelManager);

    void cleanup ();
    void reset ();

    void preRender ();
    //! Computes next stimulus and displays it on screen. Return true if not in a black phase between bar or spot stimuli.
    bool renderFrame (GLuint defaultFrameBuffer, unsigned channelId);

    void renderRandoms (Shader* randomGeneratorShader, uint32_t iStimulusFrame, uint32_t randomSeed, uint32_t freezeRandomsAfterFrame);
    void renderParticles (Shader* particleShader, uint32_t iStimulusFrame, float time);
    void renderTimeline ();
    void renderSelectedStimulusTimeline ();
    void renderSelectedStimulusTemporalKernel ();
    void renderSelectedStimulusSpatialKernel (float min, float max, float width, float height);
    void renderSelectedStimulusSpatialProfile (float min, float max, float width, float height);

    pybind11::object renderSample (uint32_t sFrame, int left, int top, int width, int height);

    std::shared_ptr<Stimulus const> getSelectedStimulus ();

    std::shared_ptr<Stimulus const> getCurrentStimulus ();
    std::shared_ptr<Response const> getCurrentResponse ();
    void                            abort ();

    unsigned int getCurrentFrame ();

    std::shared_ptr<Sequence const> getSequence ();

    //	Quad* getFullscreenQuad(){return fullscreenQuad;}
    Nothing* getNothing ();

    void pickStimulus (double x, double y);

    void raiseSignal (std::string channel);
    void clearSignal (std::string channel);

    std::shared_ptr<Ticker>    startTicker ();
    const Stimulus::SignalMap& tick (uint32_t& iTick);

    void skip (int skipCount);

    void enableExport (std::string path);
    bool exporting () const;

    uint32_t calibrationStartingFrame;
    uint32_t calibrationDuration;

    void enableCalibration (uint32_t startingFrame, uint32_t duration, float histogramMin, float histogramMax);
    void enableVideoExport (const char*, int fr, int w, int h);


    uint32_t sequenceTimelineStartFrame;
    uint32_t sequenceTimelineFrameCount;
    void setSequenceTimelineZoom (uint32_t nFrames)
    {
        sequenceTimelineFrameCount = nFrames;
    }
    void setSequenceTimelineStart (uint32_t iStartFrame)
    {
        sequenceTimelineStartFrame = iStartFrame;
    }

    uint32_t stimulusTimelineStartFrame;
    uint32_t stimulusTimelineFrameCount;
    void setStimulusTimelineZoom (uint32_t nFrames)
    {
        stimulusTimelineFrameCount = nFrames;
    }
    void setStimulusTimelineStart (uint32_t iStartFrame)
    {
        stimulusTimelineStartFrame = iStartFrame;
    }
    void pause ()
    {
        paused = !paused;
    }

    void beginCalibrationFrame (std::shared_ptr<Stimulus const> stimulus);
    void endCalibrationFrame (std::shared_ptr<Stimulus const> stimulus);

    void beginVideoExportFrame ();
    void endVideoExportFrame ();

    int getSkippedFrameCount ()
    {
        for (int q : skippedFrames)
            std::cout << q << '\t';
        return totalFramesSkipped;
    }
    std::string getSequenceTimingReport ();
    void        setText (std::string tag, std::string text)
    {
        if (text == "")
            this->text.erase (tag);
        else
            this->text[tag] = text;
    }
    void showText () { this->textVisible = !this->textVisible; }

    void updateSpatialKernel (std::shared_ptr<KernelManager> kernelManager);

    double getTime ();

    bool isShowingCursor ();
    void setResponded ();

    void setScreenResolution (uint32_t screenWidth, uint32_t screenHeight)
    {
        this->screenWidth  = screenWidth;
        this->screenHeight = screenHeight;
    }

    void toggleChannelsOrPreview ()
    {
        isDrawingPreview = !isDrawingPreview;
    }

    bool clFFT ();
};