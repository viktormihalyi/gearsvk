#pragma once

#include "core/PortHandler.h"
#include "core/Sequence.h"
#include "core/ShaderManager.h"
#include "core/StimulusRenderer.h"
#include "core/TextureManager.h"
#include "core/Ticker.h"

#include "filter/KernelManager.h"
#include "filter/fft/glFFT.h"
#include "filter/fft/openCLFFT.h"

#include "gpu/Framebuffer.hpp"
#include "gpu/Nothing.hpp"
#include "gpu/Pointgrid.hpp"
#include "gpu/Quad.hpp"
#include "gpu/RandomSequenceBuffer.hpp"
#include "gpu/Shader.hpp"
#include "gpu/StimulusGrid.hpp"
#include "gpu/Texture.hpp"
#include "gpu/TextureQueue.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#if 0
#include "FontManager.h"
#endif

//! Represents the currently active sequence. Manages resources for GPU computation.
class SequenceRenderer {
    friend class StimulusRenderer;
    friend class PassRenderer;
    friend class SpatialFilterRenderer;

    //! Active sequence.
    Sequence::CP sequence;
    Framebuffer* spatialDomainFilteringBuffers[2];

    bool                                                                                                      paused;
    unsigned int                                                                                              iFrame;
    std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::high_resolution_clock::duration> firstFrameTimePoint;
    std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::high_resolution_clock::duration> previousFrameTimePoint;
    int                                                                                                       cFrame;
    int                                                                                                       totalFramesSkipped;
    std::vector<int>                                                                                          skippedFrames;

    bool isDrawingPreview;
    int  vSyncPeriodsSinceLastFrame = -1;

    using StimulusRendererMap = std::map<unsigned int, StimulusRenderer::P>;
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
    Response::CP                       currentResponse;

    //! Index of current slice in texture queue. Changes continuously during sequence.
    unsigned int currentSlice;

    RandomSequenceBuffer* randomSequenceBuffers[5];
    std::ofstream         randomExportStream;
    RandomSequenceBuffer* particleBuffers[2];
    TextureQueue*         textureQueue;
    TextureQueue*         currentTemporalProcessingState;
    TextureQueue*         nextTemporalProcessingState;
#if 0
    FontManager fontManager;
#endif
    bool         exportingToVideo;
    uint         videoExportImageWidth;
    uint         videoExportImageHeight;
    Framebuffer* videoExportImage;
    Framebuffer* videoExportImageY;
    Framebuffer* videoExportImageU;
    Framebuffer* videoExportImageV;
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
    uint  calibrationFrameCount;
    float histogramMin;
    float histogramMax;
    float histogramScale;
    uint  histogramResolution;
    uint  calibrationImageWidth;
    uint  calibrationImageHeight;
    uint  calibrationHorizontalSampleCount;
    uint  calibrationVerticalSampleCount;

    uint screenWidth;
    uint screenHeight;

    Framebuffer* forwardRenderedImage;

    Shader*      histogramShader;
    Shader*      histogramHalferShader;
    Shader*      histogramClearShader;
    Shader*      histogramDisplayShader;
    Framebuffer* calibrationImage;
    Framebuffer* histogramBuffer;
    Framebuffer* histogramBuffer2;
    Framebuffer* histogramBuffer3;

    float measuredToneRangeMin;
    float measuredToneRangeMax;
    float measuredMean;
    float measuredVariance;

    void readCalibrationResults ();

    GLboolean _redMaskByIndex[3]   = {GL_TRUE, GL_FALSE, GL_FALSE};
    GLboolean _greenMaskByIndex[3] = {GL_FALSE, GL_TRUE, GL_FALSE};
    GLboolean _blueMaskByIndex[3]  = {GL_FALSE, GL_FALSE, GL_TRUE};

    //! Constructor. Sets some parameters to zero, but the sequence remains invalid until apply is called.
    SequenceRenderer ();

public:
    GEARS_SHARED_CREATE_WITH_GETSHAREDPTR (SequenceRenderer);

    //! Creates GPU resources for the sequence, releasing earlier ones, if any.
    void apply (Sequence::P sequence, ShaderManager::P shaderManager, TextureManager::P textureManager, KernelManager::P kernelManager);

    void cleanup ();
    void reset ();

    void preRender ();
    //! Computes next stimulus and displays it on screen. Return true if not in a black phase between bar or spot stimuli.
    bool renderFrame (GLuint defaultFrameBuffer, unsigned channelId);

    void renderRandoms (Shader* randomGeneratorShader, uint iStimulusFrame, uint randomSeed, uint freezeRandomsAfterFrame);
    void renderParticles (Shader* particleShader, uint iStimulusFrame, float time);
    void renderTimeline ();
    void renderSelectedStimulusTimeline ();
    void renderSelectedStimulusTemporalKernel ();
    void renderSelectedStimulusSpatialKernel (float min, float max, float width, float height);
    void renderSelectedStimulusSpatialProfile (float min, float max, float width, float height);

    pybind11::object renderSample (uint sFrame, int left, int top, int width, int height);

    Stimulus::CP getSelectedStimulus ()
    {
        if (selectedStimulusRenderer == stimulusRenderers.end ())
            return nullptr;
        return selectedStimulusRenderer->second->getStimulus ();
    }

    Stimulus::CP getCurrentStimulus ();
    Response::CP getCurrentResponse ();
    void         abort ();

    unsigned int getCurrentFrame ();

    Sequence::CP getSequence () { return sequence; }

    //	Quad* getFullscreenQuad(){return fullscreenQuad;}
    Nothing* getNothing () { return nothing; }

    void pickStimulus (double x, double y);

    void raiseSignal (std::string channel);
    void clearSignal (std::string channel);

    Ticker::P                  startTicker ();
    const Stimulus::SignalMap& tick (uint& iTick);

    void skip (int skipCount);

    void enableExport (std::string path);
    bool exporting () const;

    uint calibrationStartingFrame;
    uint calibrationDuration;

    void enableCalibration (uint startingFrame, uint duration, float histogramMin, float histogramMax);
    void enableVideoExport (const char*, int fr, int w, int h);


    uint sequenceTimelineStartFrame;
    uint sequenceTimelineFrameCount;
    void setSequenceTimelineZoom (uint nFrames)
    {
        sequenceTimelineFrameCount = nFrames;
    }
    void setSequenceTimelineStart (uint iStartFrame)
    {
        sequenceTimelineStartFrame = iStartFrame;
    }

    uint stimulusTimelineStartFrame;
    uint stimulusTimelineFrameCount;
    void setStimulusTimelineZoom (uint nFrames)
    {
        stimulusTimelineFrameCount = nFrames;
    }
    void setStimulusTimelineStart (uint iStartFrame)
    {
        stimulusTimelineStartFrame = iStartFrame;
    }
    void pause ()
    {
        paused = !paused;
    }

    void beginCalibrationFrame (Stimulus::CP stimulus);
    void endCalibrationFrame (Stimulus::CP stimulus);

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

    void updateSpatialKernel (KernelManager::P kernelManager);

    double getTime ();

    bool isShowingCursor ();
    void setResponded ();

    void setScreenResolution (uint screenWidth, uint screenHeight)
    {
        this->screenWidth  = screenWidth;
        this->screenHeight = screenHeight;
    }

    void toggleChannelsOrPreview ()
    {
        isDrawingPreview = !isDrawingPreview;
    }

    bool clFFT ()
    {
        return sequence->useOpenCL;
    }
};