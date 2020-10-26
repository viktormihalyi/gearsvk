#include "SequenceRenderer.h"
#include "core/pythonerr.h"
#include "stdafx.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <time.h>

#if 0
extern "C" {
#include <libavutil/opt.h>

#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#pragma warning(push)
#pragma warning(disable : 4244)
#include <libavutil/common.h>
#pragma warning(pop)
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
}
#endif

#if 0
extern PFNWGLSWAPINTERVALEXTPROC    wglSwapIntervalEXT;
extern PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;
#endif

SequenceRenderer::SequenceRenderer ()
{
    selectedStimulusRenderer = stimulusRenderers.end ();

    //	fullscreenQuad = new Quad();
    nothing = new Nothing ();

    currentResponse = nullptr;
    sequence        = nullptr;

    rgbToY         = nullptr;
    rgbToU         = nullptr;
    rgbToV         = nullptr;
    showVideoFrame = nullptr;

    textureQueue                   = nullptr;
    currentTemporalProcessingState = nullptr;
    nextTemporalProcessingState    = nullptr;

    spatialDomainFilteringBuffers[0] = nullptr;
    spatialDomainFilteringBuffers[1] = nullptr;

    randomSequenceBuffers[0] = 0;
    randomSequenceBuffers[1] = 0;
    randomSequenceBuffers[2] = 0;
    randomSequenceBuffers[3] = 0;
    randomSequenceBuffers[4] = 0;

    particleBuffers[0] = 0;
    particleBuffers[1] = 0;

    sequenceTimelineStartFrame = 0;
    sequenceTimelineFrameCount = 100;
    stimulusTimelineStartFrame = 0;
    stimulusTimelineFrameCount = 100;

    paused = false;

    calibrating = false;

    exportingToVideo = false;

    histogramMin           = -1;
    histogramMax           = 2;
    histogramScale         = 50000;
    histogramResolution    = 256;
    calibrationImageWidth  = 768;
    calibrationImageHeight = 768;
    //calibrationImageWidth = 512;
    //calibrationImageHeight = 512;
    calibrationHorizontalSampleCount = 512;
    calibrationVerticalSampleCount   = 512;

    histogramShader        = 0;
    histogramHalferShader  = 0;
    histogramClearShader   = 0;
    histogramDisplayShader = 0;
    calibrationImage       = 0;
    histogramBuffer        = 0;
    histogramBuffer2       = 0;
    histogramBuffer3       = 0;
    textShader             = 0;

    textVisible     = false;
    responseVisible = false;

    measuredToneRangeMin = std::numeric_limits<float>::quiet_NaN ();
    measuredToneRangeMax = std::numeric_limits<float>::quiet_NaN ();
    measuredMean         = std::numeric_limits<float>::quiet_NaN ();
    measuredVariance     = std::numeric_limits<float>::quiet_NaN ();

    isDrawingPreview     = false;
    forwardRenderedImage = nullptr;
    cFrame               = 1;
}

void SequenceRenderer::apply (Sequence::P sequence, ShaderManager::P shaderManager, TextureManager::P textureManager, KernelManager::P kernelManager)
{
    this->sequence  = sequence;
    iFrame          = 1;
    paused          = false;
    currentSlice    = 0;
    currentResponse = nullptr;

    rgbToY         = shaderManager->loadShaderFromFile ("./Project/Shaders/quad.vert", "./Project/Shaders/rgbToY.frag");
    rgbToU         = shaderManager->loadShaderFromFile ("./Project/Shaders/quad.vert", "./Project/Shaders/rgbToU.frag");
    rgbToV         = shaderManager->loadShaderFromFile ("./Project/Shaders/quad.vert", "./Project/Shaders/rgbToV.frag");
    showVideoFrame = shaderManager->loadShaderFromFile ("./Project/Shaders/quad.vert", "./Project/Shaders/showTexture.frag");

    if (forwardRenderedImage)
        delete forwardRenderedImage;
    if (sequence->getUsesForwardRendering ()) {
        forwardRenderedImage = new FramebufferGL (sequence->fieldWidth_px, sequence->fieldHeight_px, 1, GL_RGBA16F);
    } else
        forwardRenderedImage = nullptr;

    if (textureQueue) {
        delete textureQueue;
        textureQueue = nullptr;
    }

    if (sequence->getMaxTemporalProcessingStateCount () > 0) {
        unsigned int nSlices = sequence->getMaxTemporalProcessingStateCount () / 4 + 1;
        if (!sequence->isMonochrome ())
            nSlices *= 3;
        nSlices += 1; // output
        currentTemporalProcessingState = new TextureQueue (sequence->fieldWidth_px, sequence->fieldHeight_px, nSlices, false, false);
        currentTemporalProcessingState->clear ();
        nextTemporalProcessingState = new TextureQueue (sequence->fieldWidth_px, sequence->fieldHeight_px, nSlices, false, false);
        nextTemporalProcessingState->clear ();

        textureQueue = new TextureQueue (sequence->fieldWidth_px, sequence->fieldHeight_px, 1, sequence->isMonochrome (), false);
        textureQueue->clear ();
    } else if (sequence->getMaxMemoryLength () > 0) {
        textureQueue = new TextureQueue (sequence->fieldWidth_px, sequence->fieldHeight_px, sequence->getMaxMemoryLength (), sequence->isMonochrome (), false);
        textureQueue->clear ();
    }
    stimulusRenderers.clear ();

    for (auto& s : sequence->getStimuli ()) {
        StimulusRenderer::P stimulusRenderer = StimulusRenderer::create (getSharedPtr (), s.second, shaderManager, textureManager, kernelManager);
        stimulusRenderer->apply (shaderManager, textureManager);
        stimulusRenderers[s.first] = stimulusRenderer;
    }
    selectedStimulusRenderer = stimulusRenderers.end ();

    if (sequence->hasSpatialDomainConvolution) {
        spatialDomainFilteringBuffers[0] = new FramebufferGL (sequence->fieldWidth_px, sequence->fieldHeight_px, 1, GL_RGBA16F);
        spatialDomainFilteringBuffers[1] = new FramebufferGL (sequence->fieldWidth_px, sequence->fieldHeight_px, 1, GL_RGBA16F);
    }

    if (sequence->maxRandomGridWidth > 0) {
        randomSequenceBuffers[0] = std::make_unique<RandomSequenceBuffer> (sequence->maxRandomGridWidth, sequence->maxRandomGridHeight);
        randomSequenceBuffers[1] = std::make_unique<RandomSequenceBuffer> (sequence->maxRandomGridWidth, sequence->maxRandomGridHeight);
        randomSequenceBuffers[2] = std::make_unique<RandomSequenceBuffer> (sequence->maxRandomGridWidth, sequence->maxRandomGridHeight);
        randomSequenceBuffers[3] = std::make_unique<RandomSequenceBuffer> (sequence->maxRandomGridWidth, sequence->maxRandomGridHeight);
        randomSequenceBuffers[4] = std::make_unique<RandomSequenceBuffer> (sequence->maxRandomGridWidth, sequence->maxRandomGridHeight);
    }

    if (sequence->maxParticleGridWidth > 0) {
        particleBuffers[0] = new RandomSequenceBuffer (sequence->maxParticleGridWidth, sequence->maxParticleGridHeight);
        particleBuffers[1] = new RandomSequenceBuffer (sequence->maxParticleGridWidth, sequence->maxParticleGridHeight);
    }

    calibrating      = false;
    exportingToVideo = false;

    if (calibrationImage == 0)
        delete calibrationImage;
    if (histogramBuffer == 0)
        delete histogramBuffer;
    if (histogramBuffer2 == 0)
        delete histogramBuffer2;
    if (histogramBuffer3 == 0)
        delete histogramBuffer3;

    histogramShader        = shaderManager->loadShaderFromFile ("./Project/Shaders/computeHistogram.vert", "./Project/Shaders/computeHistogram.geom", "./Project/Shaders/computeHistogram.frag");
    histogramClearShader   = shaderManager->loadShaderFromFile ("./Project/Shaders/quad.vert", "./Project/Shaders/clearHistogram.frag");
    histogramDisplayShader = shaderManager->loadShaderFromFile ("./Project/Shaders/quad.vert", "./Project/Shaders/showHistogram.frag");
    histogramHalferShader  = shaderManager->loadShaderFromFile ("./Project/Shaders/quad.vert", "./Project/Shaders/halfHistogram.frag");
    if (sequence->getUsesDynamicToneMapping ()) {
        calibrationImageWidth  = sequence->fieldWidth_px;
        calibrationImageHeight = sequence->fieldHeight_px;
    } else {
        calibrationImageWidth  = calibrationHorizontalSampleCount;
        calibrationImageHeight = calibrationVerticalSampleCount;
    }
    calibrationImage = new FramebufferGL (calibrationImageWidth, calibrationImageHeight, 1, GL_RGBA16F);
    histogramBuffer  = new FramebufferGL (histogramResolution, 1, 1, GL_RGBA32F);
    histogramBuffer2 = new FramebufferGL (histogramResolution, 1, 1, GL_RGBA32F);
    histogramBuffer3 = new FramebufferGL (histogramResolution, 1, 1, GL_RGBA32F);
    histogramBuffer->clear ();

    videoExportImage       = nullptr;
    videoExportImageY      = nullptr;
    videoExportImageU      = nullptr;
    videoExportImageV      = nullptr;
    videoExportImageHeight = 0;
    videoExportImageWidth  = 0;

    textShader = shaderManager->loadShaderFromFile ("./Project/Shaders/text.vert", "./Project/Shaders/text.frag");

    for (auto& c : sequence->getChannels ()) {
        PortMap::iterator iPort = ports.find (c.second.portName);
        if (iPort == ports.end ()) {
            auto iNew = ports.insert (std::make_pair (c.second.portName, PortHandler (c.second.portName.c_str ())));

            if (iNew.second) {
                ports[c.second.portName].setCommand (c.second.clearFunc);
                if (ports[c.second.portName].isInvalid ()) {
                    std::stringstream ss;
                    ss << "No device on port " << c.second.portName << " !" << std::endl;
                    //throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
                    //PyErr_Warn (PyExc_Warning, ss.str ().c_str ());
                    //boost::python::throw_error_already_set();
                }
            }
        } else
            iPort->second.setCommand (c.second.clearFunc);
    }
}

void SequenceRenderer::preRender ()
{
    for (auto s : stimulusRenderers) {
        s.second->preRender ();
    }
}

bool SequenceRenderer::renderFrame (GLuint defaultFrameBuffer, unsigned channelIdx)
{
    //throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 1
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::duration<float>       Fsec;
    int                                        nSkippedFrames = 0;
    //glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    //if (sequence->useHighFreqRender) {
    //    glColorMask (_redMaskByIndex[channelIdx], _greenMaskByIndex[channelIdx], _blueMaskByIndex[channelIdx], GL_TRUE);
    //}

    //if(iFrame == 182)
    //	paused = true;
    if (iFrame >= sequence->getMeasurementStart ()) {
        //std::cout << sequence->getMeasurementStart() << ':' << iFrame << '\n';

        if (currentResponse) {
            if (iFrame > currentResponse->startingFrame + currentResponse->duration) {
                responseVisible = false;
                if (currentResponse->loop)
                    iFrame = currentResponse->startingFrame;
                std::cout << "iFrame:" << iFrame << " response start \n";
            }
        } else {
            Response::CP r = sequence->getResponseAtFrame (iFrame);
            if (r != nullptr && iFrame < r->startingFrame + r->duration) {
                currentResponse = r;
                responseVisible = true;
                std::cout << "iFrame:" << iFrame << " response end \n";
            }
        }

        if (iFrame == sequence->getMeasurementStart ()) {
            firstFrameTimePoint    = Clock::now ();
            previousFrameTimePoint = firstFrameTimePoint;
            cFrame                 = 1;
            totalFramesSkipped     = 0;
            skippedFrames.clear ();
        } else if (iFrame > sequence->getMeasurementEnd ()) {
        } else if (!calibrating && !randomExportStream.is_open () && !exportingToVideo) {
            if (channelIdx == 0) {
                // Calculate how many seconds passed since the last render
                auto now                   = Clock::now ();
                Fsec elapsed               = now - firstFrameTimePoint;
                Fsec elapsedSinceLastFrame = now - previousFrameTimePoint;
                previousFrameTimePoint     = now;

                // Calcultate how many frames we missed, because render has not been called
                vSyncPeriodsSinceLastFrame = (int)(elapsedSinceLastFrame.count () / sequence->getFrameInterval_s () + 0.5);
                if (sequence->useHighFreqRender)
                    // If we use high frequency device, than devide with 3, and use in all three channels,
                    // this way we normalize the time between frames in 1 image
                    vSyncPeriodsSinceLastFrame /= 3;
            }
            // Missed signals for missed frames
            Sequence::SignalMap::const_iterator iSignal = sequence->getSignals ().lower_bound (iFrame - 1);
            Sequence::SignalMap::const_iterator eSignal = sequence->getSignals ().upper_bound (iFrame + vSyncPeriodsSinceLastFrame);
            while (iSignal != eSignal) {
                if (iSignal->second.clear)
                    clearSignal (iSignal->second.channel);
                else
                    raiseSignal (iSignal->second.channel);
                iSignal++;
            }
            nSkippedFrames = vSyncPeriodsSinceLastFrame - 1; //nem 1, hanem count of frame / swapbuffers
            if (nSkippedFrames < 0) {
                //if(!skippedFrames.empty() && skippedFrames.back() == iFrame)
                //{
                //	skippedFrames.pop_back();
                //					}
                //else
                skippedFrames.push_back (-(int)iFrame);
            }
            totalFramesSkipped += nSkippedFrames;
            for (uint q = iFrame; q < iFrame + nSkippedFrames; q++)
                skippedFrames.push_back (q);
            iFrame += nSkippedFrames;
            cFrame += vSyncPeriodsSinceLastFrame;
        }
    } else {
        cFrame = 0;
    }

    StimulusRendererMap::iterator iStimulusRenderer = stimulusRenderers.lower_bound (iFrame);
    if (calibrating && iFrame > calibrationStartingFrame + calibrationDuration) {
        readCalibrationResults ();
        return false;
    }
    if (iStimulusRenderer == stimulusRenderers.end ()) {
#if 0
        if (exportingToVideo) {
            int i = iFrame;
            for (got_output = 1; got_output; i++) {
                fflush (stdout);
                int ret = avcodec_encode_video2 (c, &pkt, NULL, &got_output);
                if (ret < 0) {
                    fprintf (stderr, "Error encoding frame\n");
                    exit (1);
                }
                if (got_output) {
                    printf ("Write frame %3d (size=%5d)\n", i, pkt.size);
                    fwrite (pkt.data, 1, pkt.size, videoExportFile);
                    av_free_packet (&pkt);
                }
            }
            /* add sequence end code to have a real mpeg file */
            uint8_t endcode[] = {0, 0, 1, 0xb7};
            fwrite (endcode, 1, sizeof (endcode), videoExportFile);
            fclose (videoExportFile);
            avcodec_close (c);
            av_free (c);
            av_freep (frame->data);
            av_frame_free (&frame);
            delete videoExportImage;
            delete videoExportImageY;
            delete videoExportImageU;
            delete videoExportImageV;
            videoExportImage = nullptr;
            exportingToVideo = false;
        }
        return false;
#endif
    }
    StimulusRenderer::P stimulusRenderer = iStimulusRenderer->second;
    if (calibrating && iFrame == calibrationStartingFrame)
        histogramBuffer->clear ();
    Stimulus::CP stimulus = stimulusRenderer->getStimulus ();
    if (stimulus->doesDynamicToneMapping) {
        if (iFrame == stimulus->startingFrame) {
            histogramBuffer->clear ();
            calibrationFrameCount = 0;
        }
    }
    stimulusRenderer->renderStimulus (defaultFrameBuffer, nSkippedFrames);
    if (stimulus->doesDynamicToneMapping && stimulus->toneMappingMode != Stimulus::ToneMappingMode::EQUALIZED)
        readCalibrationResults ();

    if (!paused) {
        iFrame++;
        if (sequence->getMaxMemoryLength () > 0)
            currentSlice = (currentSlice + 1) % sequence->getMaxMemoryLength ();
    }

    //glViewport (sequence->fieldLeft_px, sequence->fieldBottom_px, sequence->fieldWidth_px, sequence->fieldHeight_px);

#if 0
    if (textVisible) {
        Fsec elapsed = previousFrameTimePoint - firstFrameTimePoint;
        //elapsed.count() / (cFrame-1.0);
        std::stringstream ss;
        ss << "Measured system frame rate [Hz]: " << std::setprecision (4) << (cFrame - 1.0) / elapsed.count ();
        this->setText ("__GEARS_FPS", ss.str ());

        glMatrixMode (GL_PROJECTION);
        glLoadIdentity ();
        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity ();
        glColor3d (1, 0, 0);
        glDisable (GL_TEXTURE_2D);
        glDisable (GL_LIGHTING);
        glDisable (GL_DEPTH_TEST);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        /*	glBegin(GL_QUADS);
		glVertex2d(1, 1);
		glVertex2d(1, 0);
		glVertex2d(0, 0);
		glVertex2d(0, 1);
		glEnd();*/
#ifdef _WIN32
        // TODO: linux implementation
        textShader->enable ();
        TexFont* font = fontManager.loadFont ("Candara");
        textShader->bindUniformTexture ("glyphTexture", font->getTextureId (), 0);
        glPushMatrix ();
        glTranslated (-1, 0.9, 0);
        glScaled (0.002, 0.002, 0.002);
        bool first = true;

        Stimulus::CP          stim = this->getCurrentStimulus ();
        std::set<std::string> tags = stim->getTags ();


        for (auto label : text) {
            if (tags.find (label.first) != tags.end () || label.first[0] == '_') {
                if (first)
                    font->glRenderString (label.second + "\n", "Candara", false, false, false, false, 0xefffffff, TEXFONT_MODE_OPEN_ONLY);
                else
                    font->glRenderString (label.second + "\n", "Candara", false, false, false, false, 0xefffffff, TEXFONT_MODE_CONTINUE);
                first = false;
            }
        }
        if (!first)
            glPopMatrix ();
        textShader->disable ();
        glPopMatrix ();
#endif
        glDisable (GL_BLEND);
    }
    if (currentResponse) {
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity ();
        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity ();
        glColor3d (1, 0, 0);
        glDisable (GL_TEXTURE_2D);
        glDisable (GL_LIGHTING);
        glDisable (GL_DEPTH_TEST);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef _WIN32
        // TODO: linux implementation
        TexFont* font = fontManager.loadFont ("Candara");
        textShader->bindUniformTexture ("glyphTexture", font->getTextureId (), 0);
        glPushMatrix ();
        glTranslated (-1, 0.9, 0);
        glScaled (0.002, 0.002, 0.002);
        font->glRenderString (currentResponse->question + "\n", "Candara", false, false, false, false, 0xefffffff, TEXFONT_MODE_OPEN_AND_CLOSE);
        glPopMatrix ();
#endif

        glPushMatrix ();
        glOrtho (-0.5 * sequence->fieldWidth_um,
                 0.5 * sequence->fieldWidth_um,
                 -0.5 * sequence->fieldHeight_um,
                 0.5 * sequence->fieldHeight_um,
                 0, 1);
        for (auto& button : currentResponse->buttons) {
            if (button.visible) {
                glPushMatrix ();
                glTranslated (button.xcoord, button.ycoord, 0);
                glColor3d (1, 0, 0);
                glBegin (GL_QUADS);
                glVertex2d (button.width * 0.5, button.height * 0.5);
                glVertex2d (button.width * 0.5, -button.height * 0.5);
                glVertex2d (-button.width * 0.5, -button.height * 0.5);
                glVertex2d (-button.width * 0.5, +button.height * 0.5);
                glEnd ();
                glScaled (3, 3, 3);
                glColor3d (0, 1, 0);
#ifdef __WIN32
                // TODO: linux implementaion
                glm::vec3 extent = font->getTextExtent (button.label + "\n", "Candara", false, false);
                glTranslated (-extent.x * 0.5, -extent.y * 0.5 - extent.z, 0);
                font->glRenderString (button.label + "\n", "Candara", false, false, false, false, 0xefffffff, TEXFONT_MODE_OPEN_AND_CLOSE);
#endif
                glPopMatrix ();
            }
        }
        glPopMatrix ();

        textShader->disable ();
        glPopMatrix ();
        glDisable (GL_BLEND);
    }

    if (exportingToVideo) {
        showVideoFrame->enable ();
        showVideoFrame->bindUniformTexture ("rgb", videoExportImage->getColorBuffer (0), 0);
        getNothing ()->renderQuad ();
        showVideoFrame->disable ();

        glMatrixMode (GL_PROJECTION);
        glLoadIdentity ();
        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity ();
        glColor3d (1, 0, 0);
        glDisable (GL_TEXTURE_2D);
        glDisable (GL_LIGHTING);
        glDisable (GL_DEPTH_TEST);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef __WIN32
        // TODO: linux implementaion
        textShader->enable ();
        TexFont* font = fontManager.loadFont ("Candara");
        textShader->bindUniformTexture ("glyphTexture", font->getTextureId (), 0);
        glPushMatrix ();
        glTranslated (-1, 0.9, 0);
        glScaled (0.002, 0.002, 0.002);

        font->glRenderString ("Exporting video (Frame " + std::to_string (iFrame) + "/" + std::to_string (sequence->getDuration ()) + ")", "Candara", false, false, false, false, 0xefffffff, TEXFONT_MODE_OPEN_AND_CLOSE);
        textShader->disable ();
        glPopMatrix ();
#endif
        glDisable (GL_BLEND);
    }
    if (calibrating) {
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity ();
        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity ();
        glColor3d (0, 0, 0);
        glDisable (GL_TEXTURE_2D);
        glDisable (GL_LIGHTING);
        glDisable (GL_DEPTH_TEST);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef __WIN32
        // TODO: linux implementaion
        textShader->enable ();
        TexFont* font = fontManager.loadFont ("Candara");
        textShader->bindUniformTexture ("glyphTexture", font->getTextureId (), 0);
        for (int i = 0; i <= histogramMax - histogramMin; i++) {
            int   v = i + histogramMin;
            float p = (v - histogramMin) / (histogramMax - histogramMin) * 2.0f / 1.1f - 0.909f;
            glPushMatrix ();
            glTranslated (p, -0.8, 0);
            glScaled (0.004, 0.004, 0.004);
            font->glRenderString (std::to_string (v), "Candara", false, false, false, false, 0xefffffff, TEXFONT_MODE_OPEN_AND_CLOSE);

            glPopMatrix ();
        }
        textShader->disable ();
#endif
        glDisable (GL_BLEND);


        glEnable (GL_LINE_STIPPLE);
        glLineWidth (2);
        glLineStipple (1, 0xcccc);
        glColor3d (0.6, 0.6, 0.6);
        for (int i = 0; i <= histogramMax - histogramMin; i++) {
            int   v = i + (int)histogramMin;
            float p = (v - histogramMin) / (histogramMax - histogramMin) * 2.0f / 1.1f - 0.909f;
            glPushMatrix ();
            glTranslated (p, 0, 0);

            glBegin (GL_LINES);
            glVertex2d (0, -10000000);
            glVertex2d (0, +10000000);
            glEnd ();
            glPopMatrix ();
        }
        glDisable (GL_LINE_STIPPLE);
    }

    return true;
#endif
#endif
}

void SequenceRenderer::renderTimeline ()
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());

#if 0
    glClearColor (0, 0, 0, 1);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

    /*glColor4d(0, 1, 0, 1);
	glBegin(GL_LINES);
		for(int u=0; u<8; u++)
		{
			glVertex2d(-1, u*0.25 - 1.0);
			glVertex2d(1, u*0.25 - 1.0);
		}
	glEnd();*/

    glTranslated (-1.0, 0.0, 0);
    glScaled (2.0 / sequenceTimelineFrameCount, 1.0, 1.0);
    glTranslated (-(double)sequenceTimelineStartFrame, 0.0, 0);

    if (selectedStimulusRenderer != stimulusRenderers.end ()) {
        auto stim = *selectedStimulusRenderer;
        glPushMatrix ();
        glTranslated (stim.second->getStimulus ()->getStartingFrame (), 0.0, 0);
        glColor4d (0.35, 0, 0, 1);
        glBegin (GL_QUADS);
        glVertex2d (0, -5);
        glVertex2d (0, 0.89);
        glVertex2d (stim.second->getStimulus ()->getDuration (), 0.89);
        glVertex2d (stim.second->getStimulus ()->getDuration (), -5);
        glEnd ();
        glPopMatrix ();
    }

    glColor3d (1, 0, 0);
    int cChannel = 0;
    for (auto iChannel : sequence->getChannels ()) {
        glPushMatrix ();
        std::string channelName = iChannel.first;

        glTranslated (0.0, -1, 0.0);
        glScaled (1.0, 1.0 / sequence->getChannels ().size (), 1);
        glTranslated (0.0, 0.25 + (sequence->getChannels ().size () - 1 - cChannel), 0.0);
        glScaled (1.0, 0.25, 1);

        auto& signals = sequence->getSignals ();

        glColor4d (0.3, 0, 0, 1);
        glBegin (GL_LINES);
        glVertex2d (-100, 1);
        glVertex2d (+100000, 1);
        glVertex2d (+100000, 0);
        glVertex2d (-100, 0);
        glEnd ();
        glColor4d (1, 0, 0, 1);

        bool channelHasSequenceSignal = false;
        bool lastHigh                 = false;
        glBegin (GL_LINE_STRIP);
        for (auto iSignal : signals) {
            if (iSignal.second.channel == iChannel.first) {
                if (channelHasSequenceSignal == false) {
                    channelHasSequenceSignal = true;
                    glVertex2d (1, 0);
                }
                if (iSignal.second.clear) {
                    glVertex2d (iSignal.first, lastHigh ? 1.0 : 0.0);
                    glVertex2d (iSignal.first, 0.0);
                    lastHigh = false;
                } else {
                    glVertex2d (iSignal.first, lastHigh ? 1.0 : 0.0);
                    glVertex2d (iSignal.first, 1.0);
                    lastHigh = true;
                }
            }
        }
        if (channelHasSequenceSignal)
            glVertex2d (sequence->getDuration (), lastHigh ? 1 : 0);
        glEnd ();
        glPopMatrix ();
        cChannel++;
    }

    glLineWidth (3.0);
    bool signalLevels[] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
    for (auto stim : stimulusRenderers) {
        glPushMatrix ();
        uint stimStartFrame = stim.second->getStimulus ()->getStartingFrame ();
        glTranslated (stimStartFrame, 0.0, 0.0);

        uint fromf = 0;
        if (sequenceTimelineStartFrame > stimStartFrame)
            fromf = sequenceTimelineStartFrame - stimStartFrame;
        uint dur = stim.second->getStimulus ()->getDuration ();
        uint tof = 0;
        if (sequenceTimelineStartFrame + sequenceTimelineFrameCount > stimStartFrame)
            tof = sequenceTimelineStartFrame + sequenceTimelineFrameCount - stimStartFrame;
        if (tof > dur)
            tof = dur;
        if (fromf < tof)
            stim.second->renderTimeline (signalLevels, fromf, tof - fromf);
        glPopMatrix ();
    }

    int vp[4];
    glGetIntegerv (GL_VIEWPORT, vp);

    //	// white blank for shots
    //	glViewport(vp[0], 50, vp[2], 200);
    ////	glViewport(0, 0, 1920, 400);
    //	glPushMatrix();
    //	glLoadIdentity();
    //	glColor3d(0, 0, 0);
    //	glBegin(GL_QUADS);
    //	glVertex2d(-1, -1);
    //	glVertex2d(-1, 1);
    //	glVertex2d(1, 1);
    //	glVertex2d(1, -1);
    //	glEnd();
    //	glPopMatrix();

    if (isDrawingPreview) {
        // render film strip
        glViewport (vp[0], 0, vp[2], 363);
        glPushMatrix ();
        glLoadIdentity ();
        glColor3d (0.4, 0.0, 0.0);
        glBegin (GL_QUADS);
        glVertex2d (-1, -1);
        glVertex2d (-1, 1);
        glVertex2d (1, 1);
        glVertex2d (1, -1);
        glColor3d (0, 0, 0);
        for (uint i = 0; i < 64; i++) {
            glVertex2d ((i + 0.5) / 32.0 - 1 + -0.004, -0.95 + -0.02);
            glVertex2d ((i + 0.5) / 32.0 - 1 + -0.004, -0.95 + 0.02);
            glVertex2d ((i + 0.5) / 32.0 - 1 + 0.004, -0.95 + 0.02);
            glVertex2d ((i + 0.5) / 32.0 - 1 + 0.004, -0.95 + -0.02);
        }
        for (uint i = 0; i < 64; i++) {
            glVertex2d ((i + 0.5) / 32.0 - 1 + -0.004, 0.95 + -0.02);
            glVertex2d ((i + 0.5) / 32.0 - 1 + -0.004, 0.95 + 0.02);
            glVertex2d ((i + 0.5) / 32.0 - 1 + 0.004, 0.95 + 0.02);
            glVertex2d ((i + 0.5) / 32.0 - 1 + 0.004, 0.95 + -0.02);
        }
        glEnd ();
        glPopMatrix ();

        uint iPic = 0;
        for (uint iSampleFrame = 0; iSampleFrame < sequenceTimelineFrameCount; iSampleFrame += (sequenceTimelineFrameCount - 1) / 3 + 1) {
            uint                          iFrameFrame       = sequenceTimelineStartFrame + iSampleFrame;
            StimulusRendererMap::iterator iStimulusRenderer = stimulusRenderers.lower_bound (iFrameFrame);
            if (iStimulusRenderer != stimulusRenderers.end ()) {
                iStimulusRenderer->second->renderSample (
                    iFrameFrame - iStimulusRenderer->second->getStimulus ()->getStartingFrame (),
                    80 + iPic * vp[2] / 3,
                    15,
                    vp[2] / 3,
                    333);
                iPic++;
            }
        }
        glViewport (vp[0], vp[1], vp[2], vp[3]);

        glColor4d (1, 0, 0, 1);
    }
#endif
}

void SequenceRenderer::renderSelectedStimulusTimeline ()
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());

#if 0
    glClearColor (0, 0, 0, 1);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (selectedStimulusRenderer == stimulusRenderers.end ())
        return;

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glTranslated (-1.0, 0, 0);
    glScaled (2.0 / stimulusTimelineFrameCount, 2, 1.0);
    glTranslated (-(double)stimulusTimelineStartFrame, 0.0, 0.0);

    glLineWidth (3.0);
    selectedStimulusRenderer->second->renderTimeline (nullptr, stimulusTimelineStartFrame, stimulusTimelineFrameCount);
    glColor4d (1, 0, 0, 1);
#endif
}


void SequenceRenderer::renderSelectedStimulusSpatialKernel (float min, float max, float width, float height)
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());

#if 0
    if (selectedStimulusRenderer == stimulusRenderers.end ())
        return;
    glClearColor (0, 0, 0, 1);
    glClear (GL_COLOR_BUFFER_BIT);

    selectedStimulusRenderer->second->renderSpatialKernel (min, max, width, height);
#endif
}

void SequenceRenderer::renderSelectedStimulusSpatialProfile (float min, float max, float width, float height)
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    if (selectedStimulusRenderer == stimulusRenderers.end ())
        return;
    glClearColor (0, 0, 0, 1);
    glClear (GL_COLOR_BUFFER_BIT);

    selectedStimulusRenderer->second->renderSpatialProfile (min, max, width, height);
#endif
}

void SequenceRenderer::renderSelectedStimulusTemporalKernel ()
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    if (selectedStimulusRenderer == stimulusRenderers.end ())
        return;
    glClearColor (0, 0, 0, 1);
    glClear (GL_COLOR_BUFFER_BIT);

    selectedStimulusRenderer->second->renderTemporalKernel ();
#endif
}

void SequenceRenderer::abort ()
{
    //			raise(CH0); // STOP EXPERIMENT
    //			lower(CH0); // STOP EXPERIMENT
    //			lower(CH1 | CH2 | CH3);
    //			signal = SequenceDesc::Signal::NONE;
    //			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
}

void SequenceRenderer::pickStimulus (double x, double y)
{
    uint iPickedFrame        = (int)(x * sequence->getDuration ());
    selectedStimulusRenderer = stimulusRenderers.lower_bound (iPickedFrame);
    if (selectedStimulusRenderer == stimulusRenderers.end ())
        selectedStimulusRenderer = stimulusRenderers.begin ();
}

void SequenceRenderer::raiseSignal (std::string channel)
{
    Sequence::ChannelMap::const_iterator iChannel = sequence->getChannels ().find (channel);
    if (iChannel == sequence->getChannels ().end ()) {
        //PySys_WriteStdout ("\nUnknown channel: "); //TODO
        //PySys_WriteStdout (channel.c_str ());      //TODO
        return;
    }
    PortMap::iterator iPort = ports.find (iChannel->second.portName);
    if (iPort == ports.end ()) {
        //PySys_WriteStdout ("\nChannel uses unknown port:");     //TODO
        //PySys_WriteStdout (iChannel->second.portName.c_str ()); //TODO
        return;
    }
    iPort->second.setCommand (iChannel->second.raiseFunc);
}

void SequenceRenderer::clearSignal (std::string channel)
{
    Sequence::ChannelMap::const_iterator iChannel = sequence->getChannels ().find (channel);
    if (iChannel == sequence->getChannels ().end ()) {
        //PySys_WriteStdout ("Unknown channel."); //TODO
        return;
    }
    PortMap::iterator iPort = ports.find (iChannel->second.portName);
    if (iPort == ports.end ()) {
        //PySys_WriteStdout ("Channel uses unknown port."); //TODO
        return;
    }
    iPort->second.setCommand (iChannel->second.clearFunc);
}

void SequenceRenderer::reset ()
{
    calibrating      = false;
    exportingToVideo = false;

    if (videoExportImage) {
        delete videoExportImage;
        delete videoExportImageY;
        delete videoExportImageU;
        delete videoExportImageV;
    }
    videoExportImage = nullptr;

    randomExportStream.close ();
    iFrame = 1;
    paused = false;
    //selectedStimulusRenderer = stimulusRenderers.end();
    for (auto stim : stimulusRenderers)
        stim.second->reset ();

    if (textureQueue)
        textureQueue->clear ();
    if (currentTemporalProcessingState)
        currentTemporalProcessingState->clear ();
    if (nextTemporalProcessingState)
        nextTemporalProcessingState->clear ();

    for (auto& c : sequence->getChannels ()) {
        PortMap::iterator iPort = ports.find (c.second.portName);
        if (iPort != ports.end ()) {
            iPort->second.setCommand (c.second.clearFunc);
        }
    }

    if (sequence->resetCallback)
        sequence->resetCallback ();
}

void SequenceRenderer::cleanup ()
{
    for (auto& randomSequenceBuffer : randomSequenceBuffers) {
        randomSequenceBuffer.reset ();
    }

    if (particleBuffers[0]) {
        delete particleBuffers[0];
        particleBuffers[0] = nullptr;
    }
    if (particleBuffers[1]) {
        delete particleBuffers[1];
        particleBuffers[1] = nullptr;
    }

    if (forwardRenderedImage)
        delete forwardRenderedImage;

    for (auto& iPort : ports) {
        iPort.second.close ();
    }
    if (spatialDomainFilteringBuffers[0]) {
        delete spatialDomainFilteringBuffers[0];
        spatialDomainFilteringBuffers[0] = nullptr;
    }
    if (spatialDomainFilteringBuffers[1]) {
        delete spatialDomainFilteringBuffers[1];
        spatialDomainFilteringBuffers[1] = nullptr;
    }
}

void SequenceRenderer::renderParticles (Shader* particleShader, uint iStimulusFrame, float time)
{
    particleBuffers[1]->setRenderTarget ();
    particleShader->enable ();
    particleShader->bindUniformTexture ("previousParticles", particleBuffers[0]->getColorBuffer (), 0);
    particleShader->bindUniformTexture ("randoms", randomSequenceBuffers[0]->getColorBuffer (), 1);
    particleShader->bindUniformInt ("frame", iStimulusFrame);
    particleShader->bindUniformFloat ("time", time);
    getNothing ()->renderQuad ();
    particleShader->disable ();
    particleBuffers[1]->disableRenderTarget ();

    std::swap (particleBuffers[0], particleBuffers[1]);
}

void SequenceRenderer::renderRandoms (Shader* randomGeneratorShader, uint iStimulusFrame, uint randomSeed, uint freezeRandomsAfterFrame)
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());

#if 0
    if (freezeRandomsAfterFrame != 0 && freezeRandomsAfterFrame < iStimulusFrame)
        return;
    randomSequenceBuffers[4]->setRenderTarget ();
    randomGeneratorShader->enable ();
    randomGeneratorShader->bindUniformTexture ("previousSequenceElements0",
                                               randomSequenceBuffers[0]->getColorBuffer (), 0);
    randomGeneratorShader->bindUniformTexture ("previousSequenceElements1",
                                               randomSequenceBuffers[1]->getColorBuffer (), 1);
    randomGeneratorShader->bindUniformTexture ("previousSequenceElements2",
                                               randomSequenceBuffers[2]->getColorBuffer (), 2);
    randomGeneratorShader->bindUniformTexture ("previousSequenceElements3",
                                               randomSequenceBuffers[3]->getColorBuffer (), 3);
    randomGeneratorShader->bindUniformInt ("frame", iStimulusFrame);
    randomGeneratorShader->bindUniformUint ("seed", randomSeed);
    getNothing ()->renderQuad ();
    randomGeneratorShader->disable ();
    randomSequenceBuffers[4]->disableRenderTarget ();

    // 01234
    // 01243
    // 01423
    // 04123
    // 40123

    std::swap (randomSequenceBuffers[3], randomSequenceBuffers[4]);
    std::swap (randomSequenceBuffers[2], randomSequenceBuffers[3]);
    std::swap (randomSequenceBuffers[1], randomSequenceBuffers[2]);
    std::swap (randomSequenceBuffers[0], randomSequenceBuffers[1]);

    if (randomExportStream.is_open ()) {
        glBindTexture (GL_TEXTURE_2D, randomSequenceBuffers[0]->getColorBuffer ());

        uint* randoms = new uint[sequence->maxRandomGridWidth * sequence->maxRandomGridHeight * 4];
        glGetTexImage (GL_TEXTURE_2D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, (void*)randoms);

        if (sequence->exportRandomsWithHashmark) {
            randomExportStream << "#Frame " << iStimulusFrame << " of stimulus, frame " << iFrame << " of sequence.\n";
            randomExportStream << "#Number of rows: " << sequence->maxRandomGridHeight << "\n";
            randomExportStream << "#Number of cells per row: " << sequence->maxRandomGridWidth << "\n";
            randomExportStream << "#Number of values per cell: " << sequence->exportRandomsChannelCount << "\n";
            if (sequence->exportRandomsAsReal)
                randomExportStream << "#Values are floating point in [0,1]\n";
            else if (sequence->exportRandomsAsBinary)
                randomExportStream << "#Values are either 0 or 1\n";
            else
                randomExportStream << "#Values are unsigned 32-bit integers";
        }
        uint i = 0;
        for (uint iRow = 0; iRow < sequence->maxRandomGridHeight; iRow++) {
            for (uint iPix = 0; iPix < sequence->maxRandomGridWidth; iPix++) {
                for (uint iColor = 0; iColor < sequence->exportRandomsChannelCount; iColor++, i++) {
                    if (sequence->exportRandomsAsReal)
                        randomExportStream << ((double)randoms[i] / 0xffffffff) << "\t";
                    else if (sequence->exportRandomsAsBinary)
                        randomExportStream << (randoms[i] >> 31) << "\t";
                    else
                        randomExportStream << randoms[i] << "\t";
                }
                i += 4 - sequence->exportRandomsChannelCount;
                randomExportStream << "\t";
            }
            randomExportStream << "\n";
        }
        randomExportStream << "\n\n";
        delete randoms;
    }
#endif
}

void SequenceRenderer::enableExport (std::string path)
{
    std::filesystem::path bpath (path);
    if (!std::filesystem::exists (bpath.parent_path ()))
        std::filesystem::create_directories (bpath.parent_path ());

    std::stringstream ss;
    std::time_t       t = time (0); // get time now
#ifdef _WIN32
    struct std::tm now;
    localtime_s (&now, &t);
    ss << path << "_";

    ss << (now.tm_year + 1900) << '-'
       << (now.tm_mon + 1) << '-'
       << now.tm_mday << "_" << now.tm_hour << "-" << now.tm_min << ".txt";

#elif __linux__
    struct std::tm* now;
    now = localtime (&t);
    ss << path << "_";

    ss << (now->tm_year + 1900) << '-'
       << (now->tm_mon + 1) << '-'
       << now->tm_mday << "_" << now->tm_hour << "-" << now->tm_min << ".txt";
#endif

    randomExportStream.open (ss.str ());

    if (sequence->exportRandomsWithHashmark) {
        randomExportStream << "# " << sequence->getDuration () << " frames to follow."
                           << ".\n";
    }
}

Ticker::P SequenceRenderer::startTicker ()
{
    Ticker::P ticker = Ticker::create (getSharedPtr ());
    ticker->start (sequence->tickInterval, sequence->getFrameInterval_s ());
    return ticker;
}


const Stimulus::SignalMap& SequenceRenderer::tick (uint& iTick)
{
    if (iFrame == 1)
        return noTickSignal;
    StimulusRendererMap::iterator iStimulusRenderer = stimulusRenderers.lower_bound (iFrame - 1);
    if (iStimulusRenderer == stimulusRenderers.end ())
        return noTickSignal;
    StimulusRenderer::P stimulusRenderer = iStimulusRenderer->second;
    iTick                                = stimulusRenderer->tick ();
    return stimulusRenderer->getStimulus ()->tickSignals;
}

void SequenceRenderer::skip (int skipCount)
{
    if (skipCount >= 100000000) {
        if (paused)
            paused = false;
    }
    bool measurementStarted = iFrame > sequence->getMeasurementStart ();
    if (!measurementStarted && skipCount > 1 || iFrame > sequence->getMeasurementEnd ()) {
        iFrame = sequence->getDuration ();
        return;
    }
    StimulusRendererMap::iterator iStimulusRenderer = stimulusRenderers.lower_bound (iFrame);
    if (skipCount > 0)
        for (int i = 0; i < skipCount - 1 && iStimulusRenderer != stimulusRenderers.end (); i++) {
            iStimulusRenderer++;
        }
    if (skipCount < 0)
        for (int i = 0; i < -skipCount + 1 && iStimulusRenderer != stimulusRenderers.begin (); i++) {
            iStimulusRenderer--;
        }
    if (iStimulusRenderer != stimulusRenderers.end ())
        iFrame = iStimulusRenderer->first + 1;
    else
        iFrame = sequence->getDuration ();
    if (measurementStarted && iFrame < sequence->getMeasurementStart ())
        iFrame = sequence->getMeasurementStart ();
    if (iFrame > sequence->getMeasurementEnd ())
        iFrame = sequence->getMeasurementEnd ();
}

bool SequenceRenderer::exporting () const
{
    return randomExportStream.is_open ();
}

void SequenceRenderer::beginCalibrationFrame (Stimulus::CP stimulus)
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    if (calibrating || stimulus->doesDynamicToneMapping) {
        glViewport (
            0,
            0,
            calibrationImageWidth,
            calibrationImageHeight);

        calibrationImage->setRenderTarget ();
    }
#endif
}

void SequenceRenderer::beginVideoExportFrame ()
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    if (exportingToVideo) {
        if (videoExportImage == nullptr) {
            videoExportImage  = new FramebufferGL (videoExportImageWidth, videoExportImageHeight, 1, GL_RGBA16F);
            videoExportImageY = new FramebufferGL (videoExportImageWidth, videoExportImageHeight, 1, GL_R8);
            videoExportImageU = new FramebufferGL (videoExportImageWidth / 2, videoExportImageHeight / 2, 1, GL_R8);
            videoExportImageV = new FramebufferGL (videoExportImageWidth / 2, videoExportImageHeight / 2, 1, GL_R8);
        }

        glViewport (
            0,
            0,
            videoExportImageWidth,
            videoExportImageHeight);

        videoExportImage->setRenderTarget ();
    }
#endif
}

void SequenceRenderer::endVideoExportFrame ()
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());

#if 0
    if (exportingToVideo) // add to histogram
    {
        videoExportImage->disableRenderTarget ();
        videoExportImageY->setRenderTarget ();
        rgbToY->enable ();
        rgbToY->bindUniformTexture ("rgb", videoExportImage->getColorBuffer (0), 0);
        getNothing ()->renderQuad ();
        rgbToY->disable ();
        videoExportImageY->disableRenderTarget ();
        videoExportImageU->setRenderTarget ();
        rgbToU->enable ();
        rgbToU->bindUniformTexture ("rgb", videoExportImage->getColorBuffer (0), 0);
        getNothing ()->renderQuad ();
        rgbToU->disable ();
        videoExportImageU->disableRenderTarget ();
        videoExportImageV->setRenderTarget ();
        rgbToV->enable ();
        rgbToV->bindUniformTexture ("rgb", videoExportImage->getColorBuffer (0), 0);
        getNothing ()->renderQuad ();
        rgbToV->disable ();
        videoExportImageV->disableRenderTarget ();

        glFlush ();

        glBindTexture (GL_TEXTURE_2D, videoExportImageY->getColorBuffer (0));
        glGetTexImage (GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, (void*)frame->data[0]);

        glBindTexture (GL_TEXTURE_2D, videoExportImageU->getColorBuffer (0));
        glGetTexImage (GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, (void*)frame->data[1]);
        glBindTexture (GL_TEXTURE_2D, videoExportImageV->getColorBuffer (0));
        glGetTexImage (GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, (void*)frame->data[2]);

        /*	int y, x;
			for (y = 0; y<c->height; y++) {
				for (x = 0; x<c->width; x++) {
					frame->data[0][y * frame->linesize[0] + x] = x + y + iFrame * 3;
				}
			}*/
        //	Cb and Cr
        /*for (y = 0; y<c->height / 2; y++) {
					for (x = 0; x<c->width / 2; x++) {
						frame->data[1][y * frame->linesize[1] + x] = 128 + y + iFrame * 2;
						frame->data[2][y * frame->linesize[2] + x] = 64 + x + iFrame * 5;
					}
				}*/
        av_init_packet (&pkt);
        pkt.data = NULL; // packet data will be allocated by the encoder
        pkt.size = 0;
        fflush (stdout);
        frame->pts = iFrame;
        /* encode the image */
        int ret = avcodec_encode_video2 (c, &pkt, frame, &got_output);
        if (ret < 0) {
            fprintf (stderr, "Error encoding frame\n");
            exit (1);
        }
        if (got_output) {
            printf ("Write frame %3d (size=%5d)\n", iFrame, pkt.size);
            fwrite (pkt.data, 1, pkt.size, videoExportFile);
            av_free_packet (&pkt);
        }
    }
#endif
}


void SequenceRenderer::endCalibrationFrame (Stimulus::CP stimulus)
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());

#if 0
    if (calibrating || stimulus->doesDynamicToneMapping) // add to histogram
    {
        calibrationFrameCount++;
        calibrationImage->disableRenderTarget ();

        histogramBuffer3->clear ();
        glEnable (GL_BLEND);
        glBlendFunc (GL_ONE, GL_ONE);
        histogramBuffer3->setRenderTarget ();
        histogramShader->enable ();
        histogramShader->bindUniformTexture ("inputBuffer", calibrationImage->getColorBuffer (0), 0);
        histogramShader->bindUniformFloat ("histogramLevels", (float)histogramResolution);
        histogramShader->bindUniformFloat ("domainMin", histogramMin);
        histogramShader->bindUniformFloat ("domainMax", histogramMax);
        histogramShader->bindUniformUint2 ("sampleCount", calibrationHorizontalSampleCount, calibrationVerticalSampleCount);
        nothing->renderPoints (calibrationHorizontalSampleCount * calibrationVerticalSampleCount);
        histogramShader->disable ();
        histogramBuffer3->disableRenderTarget ();
        glDisable (GL_BLEND);

        histogramBuffer2->setRenderTarget ();
        histogramHalferShader->enable ();
        histogramHalferShader->bindUniformTexture ("histogramBuffer", histogramBuffer->getColorBuffer (0), 0);
        histogramHalferShader->bindUniformTexture ("frameHistogramBuffer", histogramBuffer3->getColorBuffer (0), 1);
        if (stimulus->computesFullAverageForHistogram || calibrationFrameCount == 1) {
            histogramHalferShader->bindUniformFloat ("oldFramesWeight", (calibrationFrameCount - 1.0f) / calibrationFrameCount);
            histogramHalferShader->bindUniformFloat ("newFrameWeight", 1.0f / calibrationFrameCount);
        } else {
            histogramHalferShader->bindUniformFloat ("oldFramesWeight", stimulus->histogramMeasurementImpedance);
            histogramHalferShader->bindUniformFloat ("newFrameWeight", 1.0f - stimulus->histogramMeasurementImpedance);
        }
        nothing->renderQuad ();
        histogramHalferShader->disable ();
        histogramBuffer2->disableRenderTarget ();

        std::swap (histogramBuffer, histogramBuffer2);

        //glFinish();
        glViewport (
            0,
            0,
            screenWidth,
            screenHeight);

        if (calibrating) {
            histogramDisplayShader->enable ();
            histogramDisplayShader->bindUniformTexture ("histogramBuffer", histogramBuffer->getColorBuffer (0), 0);
            histogramDisplayShader->bindUniformFloat ("maxValue", histogramScale * (iFrame - calibrationStartingFrame));
            histogramDisplayShader->bindUniformFloat ("domainMin", histogramMin);
            histogramDisplayShader->bindUniformFloat ("domainMax", histogramMax);
            nothing->renderQuad ();
            histogramDisplayShader->disable ();
        }
    }
#endif
}

void SequenceRenderer::enableVideoExport (const char* path, int fr, int w, int h)
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    std::filesystem::path bpath (path);
    if (!std::filesystem::exists (bpath.parent_path ()))
        std::filesystem::create_directories (bpath.parent_path ());

    videoExportImageHeight = h;
    videoExportImageWidth  = w;

    exportingToVideo = true;

    AVCodec* codec;

    uint8_t endcode[] = {0, 0, 1, 0xb7};
    printf ("Encode video file %s\n", path);

    avcodec_register_all ();
    /* find the mpeg1 video encoder */
    //	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    codec = avcodec_find_encoder (AV_CODEC_ID_MPEG2VIDEO);
    //	codec = avcodec_find_encoder(AV_CODEC_ID_RAWVIDEO);
    if (!codec) {
        fprintf (stderr, "Codec not found\n");
        exit (1);
    }
    c = avcodec_alloc_context3 (codec);
    if (!c) {
        fprintf (stderr, "Could not allocate video codec context\n");
        exit (1);
    }
    /* put sample parameters */
    c->bit_rate = 40000000;
    //c->bit_rate_tolerance = 40000000;
    /* resolution must be a multiple of two */
    c->width  = videoExportImageWidth;
    c->height = videoExportImageHeight;
    /* frames per second */
    c->time_base    = AVRational {1, 60};
    c->gop_size     = 10; /* emit one intra frame every ten frames */
    c->max_b_frames = 1;
    //	c->pix_fmt = AV_PIX_FMT_RGB8;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    /* open it */
    if (avcodec_open2 (c, codec, NULL) < 0) {
        fprintf (stderr, "Could not open codec\n");
        exit (1);
    }
    avcodec_align_dimensions (c, &(int&)videoExportImageWidth, &(int&)videoExportImageHeight);
    c->width  = videoExportImageWidth;
    c->height = videoExportImageHeight;

#ifdef _WIN32
    auto result = fopen_s (&videoExportFile, path, "wb");
    if (result) {
        fprintf (stderr, "Could not open %s\n", path);
        exit (1);
    }
#elif __linux__
    videoExportFile = fopen (path, "wb");
    if (videoExportFile == nullptr) {
        fprintf (stderr, "Could not open %s\n", path);
        exit (1);
    }
#endif
    frame = av_frame_alloc ();
    if (!frame) {
        fprintf (stderr, "Could not allocate video frame\n");
        exit (1);
    }
    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;

    int res = av_image_alloc (frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);

    //video_encode_example(path, AV_CODEC_ID_MPEG1VIDEO);
#endif
}

void SequenceRenderer::enableCalibration (uint startingFrame, uint duration, float histogramMin, float histogramMax)
{
    calibrating              = true;
    calibrationStartingFrame = startingFrame;
    calibrationDuration      = duration;
    this->histogramMin       = histogramMin;
    this->histogramMax       = histogramMax;

    //	histogramBuffer->clear();
    //	histogramBuffer->setRenderTarget();
    //	histogramClearShader->enable();
    //	nothing->renderQuad();
    //	histogramClearShader->disable();
    //	histogramBuffer->disableRenderTarget();

    iFrame                = calibrationStartingFrame;
    calibrationFrameCount = 0;
}


void SequenceRenderer::readCalibrationResults ()
{
    throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());

#if 0
    glBindTexture (GL_TEXTURE_2D, histogramBuffer->getColorBuffer (0));

    float* ohisti = new float[histogramResolution * 4];
    float* histi  = new float[histogramResolution * 4];

    glGetTexImage (GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, (void*)ohisti);

    float runningVal = 0.f;
    for (uint e = 0; e < histogramResolution; e++) {
        float temp   = ohisti[e * 4] - runningVal;
        runningVal   = ohisti[e * 4];
        histi[e * 4] = temp;
    }
    runningVal = 0.f;
    for (uint e = 0; e < histogramResolution; e++) {
        float temp       = ohisti[e * 4 + 1] - runningVal;
        runningVal       = ohisti[e * 4 + 1];
        histi[e * 4 + 1] = temp;
    }
    runningVal = 0.f;
    for (uint e = 0; e < histogramResolution; e++) {
        float temp       = ohisti[e * 4 + 2] - runningVal;
        runningVal       = ohisti[e * 4 + 2];
        histi[e * 4 + 2] = temp;
    }
    runningVal = 0.f;
    for (uint e = 0; e < histogramResolution; e++) {
        float temp       = ohisti[e * 4 + 3] - runningVal;
        runningVal       = ohisti[e * 4 + 3];
        histi[e * 4 + 3] = temp;
    }

    float  histogramTop = 0;
    double m            = 0;
    double w            = 0;
    for (uint e = 0; e < histogramResolution; e++) {
        double y = histi[e * 4] + histi[e * 4 + 1] + histi[e * 4 + 2];
        w += y;
        m += y * (e / (float)histogramResolution * (histogramMax - histogramMin) + histogramMin);
        histogramTop = std::max (histogramTop, histi[e * 4]);
    }
    measuredMean   = (float)(m / w);
    histogramScale = histogramTop / (float)(iFrame - calibrationStartingFrame) * 1000.4f;

    double vari2 = 0;
    for (uint e = 0; e < histogramResolution; e++) {
        double d = e / (float)histogramResolution * (histogramMax - histogramMin) + histogramMin - measuredMean;
        vari2 += (histi[e * 4] + histi[e * 4 + 1] + histi[e * 4 + 2]) * d * d;
    }
    measuredVariance = (float)sqrt (vari2 / w);

    for (uint e = 0; e < histogramResolution; e++) {
        if (histi[e * 4] + histi[e * 4 + 1] + histi[e * 4 + 2] > 10.5) {
            measuredToneRangeMin = e / (float)histogramResolution * (histogramMax - histogramMin) + histogramMin;
            break;
        }
    }
    for (uint e = 0; e < histogramResolution - 1; e++) {
        if (histi[(histogramResolution - 1 - e) * 4] + histi[(histogramResolution - 1 - e) * 4 + 1] + histi[(histogramResolution - 1 - e) * 4 + 2] > 10.5) {
            measuredToneRangeMax = (histogramResolution - e) / (float)histogramResolution * (histogramMax - histogramMin) + histogramMin;
            break;
        }
    }

    StimulusRendererMap::iterator iStimulusRenderer = stimulusRenderers.lower_bound (calibrationStartingFrame);
    if (iStimulusRenderer == stimulusRenderers.end ()) {
        delete histi;
        delete ohisti;
        return;
    }
    iStimulusRenderer->second->getStimulus ()->setMeasuredDynamics (measuredToneRangeMin, measuredToneRangeMax, measuredMean, measuredVariance, ohisti, histogramResolution);
    delete histi;
    delete ohisti;
#endif
}

Stimulus::CP SequenceRenderer::getCurrentStimulus ()
{
    StimulusRendererMap::iterator iStimulusRenderer = stimulusRenderers.lower_bound (iFrame);
    if (iStimulusRenderer == stimulusRenderers.end ())
        iStimulusRenderer = stimulusRenderers.begin ();
    return iStimulusRenderer->second->getStimulus ();
}

Response::CP SequenceRenderer::getCurrentResponse ()
{
    return sequence->getResponseAtFrame (iFrame);
}

std::string SequenceRenderer::getSequenceTimingReport ()
{
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::duration<float>       Fsec;

    std::stringstream ss;
    Fsec              elapsed = previousFrameTimePoint - firstFrameTimePoint;

    if (calibrating || randomExportStream.is_open ())
        ss << "Frame rate not measured during histogram measurement or random number export.<BR>";
    else if (cFrame == 0)
        ss << "Sequence aborted before measurement start.<BR>";
    else {
        ss << "Total measurement duration: " << elapsed.count () << "<BR>";
        ss << "Frames dropped/total: " << totalFramesSkipped << '/' << cFrame - 1 << "<BR>";
        ss << "Measured system frame interval [s]: " << elapsed.count () / (cFrame - 1.0) << "<BR>";
        ss << "Measured system frame rate [Hz]: " << (cFrame - 1.0) / elapsed.count () << "<BR>";
    }
    return ss.str ();
}

void SequenceRenderer::updateSpatialKernel (KernelManager::P kernelManager)
{
    kernelManager->update (getCurrentStimulus ()->getSpatialFilter ());
}

double SequenceRenderer::getTime ()
{
    return iFrame * sequence->getFrameInterval_s ();
}


pybind11::object SequenceRenderer::renderSample (uint sFrame, int left, int top, int width, int height)
{
    StimulusRendererMap::iterator iStimulusRenderer = stimulusRenderers.lower_bound (sFrame);
    if (iStimulusRenderer == stimulusRenderers.end ()) {
        return pybind11::object ();
    }
    iStimulusRenderer->second->renderSample (sFrame - iStimulusRenderer->second->getStimulus ()->getStartingFrame (), left, top, width, height);
    return iStimulusRenderer->second->getStimulus ()->getPythonObject ();
}

bool SequenceRenderer::isShowingCursor ()
{
    return currentResponse != nullptr || textVisible;
}

void SequenceRenderer::setResponded ()
{
    iFrame          = currentResponse->startingFrame + currentResponse->duration;
    currentResponse = nullptr;
}