#include "StimulusRenderer.h"

#include "gpu/Framebuffer.hpp"
#include "gpu/Nothing.hpp"
#include "gpu/Pointgrid.hpp"
#include "gpu/Quad.hpp"
#include "gpu/RandomSequenceBuffer.hpp"
#include "gpu/Shader.hpp"
#include "gpu/StimulusGrid.hpp"
#include "gpu/Texture.hpp"
#include "gpu/TextureQueue.hpp"


#include "core/PassRenderer.h"
#include "core/ShaderManager.h"
#include "core/Stimulus.h"
#include "core/TextureManager.h"
#include "core/filter/SpatialFilterRenderer.h"
#include "filter/SpatialFilter.h"

#include "SequenceRenderer.h"
#include "core/pythonerr.h"
#include "event/events.h"
#include "filter/SpatialFilterRenderer.h"
#include "filter/clSpatialFilterRenderer.h"
#include "filter/glSpatialFilterRenderer.h"
#include "stdafx.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

StimulusRenderer::StimulusRenderer (std::shared_ptr<SequenceRenderer> sequenceRenderer, std::shared_ptr<Stimulus const> stimulus, std::shared_ptr<ShaderManager> shaderManager, std::shared_ptr<TextureManager> textureManager, std::shared_ptr<KernelManager> kernelManager)
    : stimulus (stimulus)
    , sequenceRenderer (sequenceRenderer)
{
    randomGeneratorShader = shaderManager->loadShader (stimulus->getRandomGeneratorShaderSource ());
    particleShader        = shaderManager->loadShader (stimulus->getParticleShaderSource ());

    iFrame = 1;
    iTick  = 1;

    temporalFilteringShader = shaderManager->loadShader (stimulus->getTemporalFilterShaderSource ());
    if (stimulus->doesDynamicToneMapping)
        dynamicToneShader = shaderManager->loadShader (stimulus->getDynamicToneShaderSource ());
    else
        dynamicToneShader = nullptr;

    // throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
    if (stimulus->spatialFilter) {
        GVK_BREAK ("spatial filter");
#if 0
        auto width  = sequenceRenderer->getSequence ()->fftWidth_px;
        auto height = sequenceRenderer->getSequence ()->fftHeight_px;
        if (sequenceRenderer->clFFT ()) {
            FFTChannelMode mode   = sequenceRenderer->getSequence ()->isMonochrome () ? FFTChannelMode::Monochrome : FFTChannelMode::Multichrome;
            spatialFilterRenderer = make_unique<CLSpatialFilterRenderer> (std::shared_ptr<SequenceRenderer> (sequenceRenderer), shaderManager, kernelManager, stimulus->spatialFilter, width, height, mode);
        } else {
            spatialFilterRenderer = make_unique<GLSpatialFilterRenderer> (std::shared_ptr<SequenceRenderer> (sequenceRenderer), shaderManager, kernelManager, stimulus->spatialFilter, width, height);
        }
        std::string fragmentShaderSource = stimulus->spatialFilter->getKernelGeneratorShaderSource ();
        kernelShader                     = shaderManager->loadShader (fragmentShaderSource);
        profileShader                    = shaderManager->loadShader (
            stimulus->spatialFilter->getProfileVertexShaderSource (),
            stimulus->spatialFilter->getProfileFragmentShaderSource ());
#endif
    } else {
        kernelShader  = nullptr;
        profileShader = nullptr;
    }


    temporalProfileShader = shaderManager->loadShader (
        stimulus->getTemporalFilterPlotVertexShaderSource (),
        stimulus->getTemporalFilterPlotFragmentShaderSource ());


    spikeShader = shaderManager->loadShader (
        stimulus->spikeVertexShaderSource,
        stimulus->spikeFragmentShaderSource);

    gammaTexture = new Texture1D ();
    gammaTexture->initialize (256);
    float gammaAndTemporalWeights[256];
    for (int i = 0; i < 101; i++)
        gammaAndTemporalWeights[i] = stimulus->gamma[i];
    for (int i = 0; i < 64; i++)
        gammaAndTemporalWeights[128 + i] = stimulus->temporalWeights[i];
    gammaTexture->setData (gammaAndTemporalWeights);

    measuredHistogramTexture = nullptr;
    if (stimulus->toneMappingMode == Stimulus::ToneMappingMode::EQUALIZED && !stimulus->doesDynamicToneMapping && stimulus->measuredHistogram.size () > 0) {
        measuredHistogramTexture = new Texture2D ();
        measuredHistogramTexture->initialize (stimulus->measuredHistogram.size (), 1, 32);
        measuredHistogramTexture->setData (&stimulus->measuredHistogram[0]);
    }

    passRenderers.clear ();
}

void StimulusRenderer::apply (std::shared_ptr<ShaderManager> shaderManager, std::shared_ptr<TextureManager> textureManager)
{
    for (auto& p : stimulus->getPasses ()) {
        std::shared_ptr<PassRenderer> passRenderer = PassRenderer::create (getSharedPtr (), p, shaderManager, textureManager);
        passRenderers.push_back (passRenderer);
    }
}

StimulusRenderer::~StimulusRenderer ()
{
    delete gammaTexture;
}

void StimulusRenderer::preRender ()
{
    throw std::runtime_error (Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString ());
    /*
    if (sequenceRenderer->clFFT () && spatialFilterRenderer) {
        std::function<void (int)> stim = [&] (int frame) {
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            if (stimulus->requiresClearing) {
                glClearColor (stimulus->clearColor.x, stimulus->clearColor.y, stimulus->clearColor.z, 1.0);
                glClear (GL_COLOR_BUFFER_BIT);
            }

            for (auto& passRenderer : passRenderers) {
                passRenderer->renderSample (frame);
                glEnable (GL_BLEND);
                glBlendFunc (GL_ONE, GL_ONE);
            }
            glDisable (GL_BLEND);
        };
        spatialFilterRenderer->initFirstFrames (stim);
    }
    */
}

void StimulusRenderer::renderStimulus (GLuint defaultFrameBuffer, int skippedFrames)
{
    //throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
#if 0
    {
        int err = glGetError ();
        if (err)
            bool kmaugaz = true;
    }

    typedef std::chrono::high_resolution_clock Clock;
    auto                                       now  = Clock::now ();
    auto                                       prev = Clock::now ();

    float time = stimulus->sequence->getTimeForFrame (getCurrentFrame ());

    if (!sequenceRenderer->getSequence ()->getUsesBusyWaitingThreadForSingals ()) {
        Stimulus::SignalMap::const_iterator iSignal = stimulus->getSignals ().lower_bound (iFrame - 1);
        Stimulus::SignalMap::const_iterator eSignal = stimulus->getSignals ().upper_bound (iFrame + skippedFrames + 1);
        bool                                handled = false;
        while (iSignal != eSignal) {
            if (iSignal->second.clear)
                sequenceRenderer->clearSignal (iSignal->second.channel);
            else
                sequenceRenderer->raiseSignal (iSignal->second.channel);
            iSignal++;
            handled = true;
        }
        if (!handled) {
            Stimulus::SignalMap::const_iterator iSignal = stimulus->getSignals ().find (0);
            while (iSignal != stimulus->getSignals ().end () && iSignal->first == 0) {
                if (iSignal->second.clear)
                    sequenceRenderer->clearSignal (iSignal->second.channel);
                else
                    sequenceRenderer->raiseSignal (iSignal->second.channel);
                iSignal++;
            }
        }
    }
    //TODO manage skipped frames in some consistent manner. With PRNGs just skipping a frame is not really tolerable
    //iFrame += skippedFrames;

    if (!sequenceRenderer->paused) {
        try {
            Gears::Event::std::shared_ptr<Frame> frameEvent = Gears::Event::Frame::create (iFrame, time);
            if (iFrame == 1)
                stimulus->executeCallbacks<Gears::Event::StimulusStart> (Gears::Event::StimulusStart::create ());

            stimulus->executeCallbacks<Gears::Event::Frame> (frameEvent);

            if (iFrame == stimulus->getDuration ())
                stimulus->executeCallbacks<Gears::Event::StimulusEnd> (Gears::Event::StimulusEnd::create ());
        } catch (boost::python::error_already_set e) {
            if (PyErr_Occurred ()) {
                std::stringstream ss;
                ss << "Runtime error in callback function: " << pythonerrAsString () << " !" << std::endl;
                PyErr_Warn (PyExc_Warning, ss.str ().c_str ());
            }
            boost::python::handle_exception ();
            PyErr_Clear ();
        }

        if (stimulus->randomSeed != 0) {
            sequenceRenderer->renderRandoms (randomGeneratorShader, iFrame, stimulus->randomSeed, stimulus->freezeRandomsAfterFrame);
        }

        if (stimulus->particleGridWidth != 0) {
            sequenceRenderer->renderParticles (particleShader, iFrame, iFrame * sequenceRenderer->getSequence ()->getFrameInterval_s ());
        }
    }
    if (stimulus->usesForwardRendering) {    
        sequenceRenderer->forwardRenderedImage->setRenderTarget ();
        int err = glGetError ();
        glViewport (0, 0, stimulus->sequence->fieldWidth_px, stimulus->sequence->fieldHeight_px);
        err = glGetError ();
        stimulus->forwardRenderingCallback (iFrame);
        err = glGetError ();
        sequenceRenderer->forwardRenderedImage->disableRenderTarget ();
        
    }
    auto stim = [&] (int offset) {
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        if (stimulus->requiresClearing) {
            glClearColor (stimulus->clearColor.x, stimulus->clearColor.y, stimulus->clearColor.z, 1.0);
            glClear (GL_COLOR_BUFFER_BIT);
        }

        for (auto& passRenderer : passRenderers) {
            passRenderer->renderPass (skippedFrames, 0);
            glEnable (GL_BLEND);
            glBlendFunc (GL_ONE, GL_ONE);
        }
        glDisable (GL_BLEND);
    };
    {
        int err = glGetError ();
        if (err)
            bool kmaugaz = true;
    }

    if (spatialFilterRenderer) {
        spatialFilterRenderer->renderFrame (stim);
    } else {
        glViewport (stimulus->sequence->fieldLeft_px, stimulus->sequence->fieldBottom_px, stimulus->sequence->fieldWidth_px, stimulus->sequence->fieldHeight_px);
        if (stimulus->sequence->getMaxTemporalProcessingStateCount () > 0) {
            sequenceRenderer->nextTemporalProcessingState->setRenderTargets ();
            stim (0);
            sequenceRenderer->nextTemporalProcessingState->disableRenderTargets ();
            std::swap (sequenceRenderer->nextTemporalProcessingState, sequenceRenderer->currentTemporalProcessingState);
        }
        //		else if(stimulus->sequence->getMaxMemoryLength() > 0)
        else if (!stimulus->doesToneMappingInStimulusGenerator) {
            sequenceRenderer->textureQueue->setRenderTarget (sequenceRenderer->currentSlice);
            stim (0);
            sequenceRenderer->textureQueue->disableRenderTarget ();
        } else {
            sequenceRenderer->beginCalibrationFrame (stimulus);
            sequenceRenderer->beginVideoExportFrame ();
            stim (0);
        }
    }

    {
        int err = glGetError ();
        if (err)
            bool kmaugaz = true;
    }

    if (!stimulus->doesToneMappingInStimulusGenerator) {
        /// FINAL STAGE: TEMPORAL FILTERING + DYNAMIC RANGE MAPPING
        glBindFramebuffer (GL_FRAMEBUFFER, defaultFrameBuffer);
        glViewport (
            stimulus->sequence->fieldLeft_px,
            stimulus->sequence->fieldBottom_px,
            stimulus->sequence->fieldWidth_px,
            stimulus->sequence->fieldHeight_px);

        sequenceRenderer->beginCalibrationFrame (stimulus);
        sequenceRenderer->beginVideoExportFrame ();

        temporalFilteringShader->enable ();
        if (stimulus->temporalProcessingStateCount > 0) {
            unsigned int nSlices = sequenceRenderer->getSequence ()->getMaxTemporalProcessingStateCount () / 4 + 1;
            if (!sequenceRenderer->getSequence ()->isMonochrome ())
                nSlices *= 3;
            nSlices += 1; // output

            temporalFilteringShader->bindUniformInt ("currentSlice", 0);
            temporalFilteringShader->bindUniformInt ("memoryLength", 1);
            temporalFilteringShader->bindUniformInt ("queueLength", nSlices);
            temporalFilteringShader->bindUniformTextureArray ("stimulusQueue", sequenceRenderer->currentTemporalProcessingState->getColorBuffer (), 0);
        } else {
            temporalFilteringShader->bindUniformInt ("currentSlice", sequenceRenderer->currentSlice);

            if (stimulus->fullScreenTemporalFiltering) {
                //temporalFilteringShader->bindUniformFloatArray("temporalWeights", stimulus->temporalWeights, 64);
                temporalFilteringShader->bindUniformInt ("memoryLength", stimulus->temporalMemoryLength);
                temporalFilteringShader->bindUniformInt ("queueLength", sequenceRenderer->getSequence ()->getMaxMemoryLength ());
            } else {
                const float w[] = {
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    1,
                };
                temporalFilteringShader->bindUniformFloatArray ("temporalWeights", w, 64);
                temporalFilteringShader->bindUniformInt ("memoryLength", 1);
            }
            temporalFilteringShader->bindUniformTextureArray ("stimulusQueue", sequenceRenderer->textureQueue->getColorBuffer (), 0);
        }

        temporalFilteringShader->bindUniformFloat ("toneRangeMin", stimulus->toneRangeMin);
        temporalFilteringShader->bindUniformFloat ("toneRangeMax", stimulus->toneRangeMax);
        if (stimulus->toneMappingMode == Stimulus::ToneMappingMode::ERF) {
            temporalFilteringShader->bindUniformFloat ("toneRangeMean", stimulus->toneRangeMean);
            temporalFilteringShader->bindUniformFloat ("toneRangeVar", stimulus->toneRangeVar);
        } else {
            temporalFilteringShader->bindUniformFloat ("toneRangeMean", 0.f);
            temporalFilteringShader->bindUniformFloat ("toneRangeVar", -1.f);
        }
        temporalFilteringShader->bindUniformBool ("doGamma", !sequenceRenderer->calibrating && !stimulus->doesDynamicToneMapping);
        temporalFilteringShader->bindUniformBool ("doTone", !stimulus->doesDynamicToneMapping);
        temporalFilteringShader->bindUniformTexture1D ("gamma", gammaTexture->getTextureHandle (), 1);
        if (measuredHistogramTexture)
            temporalFilteringShader->bindUniformTexture ("histogram", measuredHistogramTexture->getTextureHandle (), 2);
        temporalFilteringShader->bindUniformInt ("gammaSampleCount", stimulus->gammaSamplesCount);
        sequenceRenderer->getNothing ()->renderQuad ();
        temporalFilteringShader->disable ();
    }

    {
        int err = glGetError ();
        if (err)
            bool kmaugaz = true;
    }

    sequenceRenderer->endVideoExportFrame ();
    sequenceRenderer->endCalibrationFrame (stimulus);

    if (stimulus->doesDynamicToneMapping && !sequenceRenderer->calibrating) {
        dynamicToneShader->enable ();
        dynamicToneShader->bindUniformTexture ("stimulusImage", sequenceRenderer->calibrationImage->getColorBuffer (0), 0);
        dynamicToneShader->bindUniformTexture1D ("gamma", gammaTexture->getTextureHandle (), 1);
        if (stimulus->toneMappingMode == Stimulus::ToneMappingMode::LINEAR) {
            dynamicToneShader->bindUniformFloat ("toneRangeMin", stimulus->measuredToneRangeMin);
            dynamicToneShader->bindUniformFloat ("toneRangeMax", stimulus->measuredToneRangeMax);
        } else if (stimulus->toneMappingMode == Stimulus::ToneMappingMode::ERF) {
            dynamicToneShader->bindUniformFloat ("toneRangeMean", stimulus->measuredMean);
            dynamicToneShader->bindUniformFloat ("toneRangeVar", stimulus->measuredVariance);
        } else //Stimulus::ToneMappingMode::EQUALIZED
            dynamicToneShader->bindUniformTexture ("histogram", sequenceRenderer->histogramBuffer2->getColorBuffer (0), 2);
        dynamicToneShader->bindUniformInt ("gammaSampleCount", stimulus->gammaSamplesCount);
        dynamicToneShader->bindUniformFloat2 ("histogramLimits", sequenceRenderer->histogramMin, 1.0f / (sequenceRenderer->histogramMax - sequenceRenderer->histogramMin));
        sequenceRenderer->getNothing ()->renderQuad ();
        dynamicToneShader->disable ();
    }


    if (!sequenceRenderer->paused)
        iFrame++;

    {
        int err = glGetError ();
        if (err)
            bool kmaugaz = true;
    }
#endif
}

void StimulusRenderer::renderSample (uint sFrame, int left, int top, int width, int height)
{
    throw std::runtime_error (Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString ());
    /*
    float time = sFrame / stimulus->sequence->deviceFrameRate * stimulus->sequence->frameRateDivisor;

    if (stimulus->usesForwardRendering) {
        sequenceRenderer->forwardRenderedImage->setRenderTarget ();
        glViewport (left + 4, top + 4, width - 8, height - 8);
        sequenceRenderer->forwardRenderedImage->disableRenderTarget ();
    }
    auto stim = [&] () {
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        for (auto& passRenderer : passRenderers) {
            passRenderer->renderSample (sFrame);
            glEnable (GL_BLEND);
            glBlendFunc (GL_ONE, GL_ONE);
        }
        glDisable (GL_BLEND);
    };

    //TODO spatial filter
    glViewport (left + 4, top + 4, width - 8, height - 8);
    //glViewport( left, top, width, height);
    stim ();

    // blue frame
    // glPushMatrix();
    // glLoadIdentity();
    // glColor3d(0.5, 0.5, 1);
    // glLineWidth(4);
    // glBegin(GL_LINE_STRIP);
    // glVertex2d(-1, 1);
    // glVertex2d(1, 1);
    // glVertex2d(1, -1);
    // glVertex2d(-1, -1);
    // glVertex2d(-1, 1);
    // glEnd();
    // glPopMatrix();


    //TODO gamma? definitely no temporal
    */
}

void StimulusRenderer::renderTimeline (bool* signals, uint startFrame, uint frameCount)
{
    throw std::runtime_error (Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString ());
    /*

    for (auto& p : passRenderers)
        p->renderTimeline (startFrame, frameCount);

    //if(signals == nullptr)
    //	return;

    glColor4d (1, 0, 0, 1);
    glLineWidth (2.0);
    glDisable (GL_LINE_STIPPLE);
    //	std::string channelName = "Stimulus sync";

    int cChannel = 0;
    for (auto iChannel : stimulus->sequence->getChannels ()) {
        glPushMatrix ();
        std::string channelName   = iChannel.first;
        bool        localLastHigh = false;
        bool&       lastHigh      = signals ? signals[cChannel] : localLastHigh;

        glTranslated (0.0, -1, 0.0);
        glScaled (stimulus->sequence->tickInterval / stimulus->sequence->getFrameInterval_s (),
                  1.0 / stimulus->sequence->getChannels ().size (), 1);
        glTranslated (0.0, 0.25 + (stimulus->sequence->getChannels ().size () - 1 - cChannel), 0.0);
        glScaled (1.0, 0.25, 1);

        bool                                spiky   = false;
        Stimulus::SignalMap::const_iterator iSignal = stimulus->tickSignals.find (0);
        while (iSignal != stimulus->tickSignals.end () && iSignal->first == 0) //TODO: more flexible stim sync setting for chirp maybe?
        {
            if (iSignal->second.channel == channelName) {
                spiky = true;
                break;
            }
            iSignal++;
        }
        glColor4d (0.25, 0, 0, 1);
        glLineWidth (2.0);
        glEnable (GL_LINE_STIPPLE);
        glLineStipple (2, 0x5555);
        glBegin (GL_LINES);
        glVertex2d (0, 1.15);
        glVertex2d (0, -0.15);
        glEnd ();
        glDisable (GL_LINE_STIPPLE);

        glColor4d (1, 0, 0, 1);
        if (spiky) {
            glPushMatrix ();
            spikeShader->enable ();
            int scount = int (frameCount * sequenceRenderer->getSequence ()->getFrameInterval_s () / stimulus->sequence->tickInterval * 3 + 1);
            int stride = scount / 8000 + 1;
            spikeShader->bindUniformInt ("stride", stride);
            spikeShader->bindUniformInt ("startOffset", (int)(startFrame * sequenceRenderer->getSequence ()->getFrameInterval_s () / stimulus->sequence->tickInterval));
            sequenceRenderer->getNothing ()->renderLineStrip (scount / stride);
            spikeShader->disable ();
            glPopMatrix ();
            //glBegin(GL_TRIANGLE_STRIP);
            //	glVertex2d(0, 0);
            //	glVertex2d(0, 1);
            //	glVertex2d(stimulus->duration / stimulus->sequence->tickInterval * stimulus->sequence->getFrameInterval_s(), 0);
            //	glVertex2d(stimulus->duration / stimulus->sequence->tickInterval * stimulus->sequence->getFrameInterval_s(), 1);
            //glEnd();
        }
        iSignal                   = stimulus->tickSignals.upper_bound (0);
        bool channelHasTickSignal = false;
        while (iSignal != stimulus->tickSignals.end ()) {
            if (iSignal->second.channel == channelName) {
                if (channelHasTickSignal == false) {
                    glColor4d (0.25, 0, 0, 1);
                    glBegin (GL_LINES);
                    glVertex2d (0, 0);
                    glVertex2d (stimulus->getDuration_s () / stimulus->sequence->tickInterval, 0);
                    glVertex2d (0, 1);
                    glVertex2d (stimulus->getDuration_s () / stimulus->sequence->tickInterval, 1);
                    glEnd ();

                    glColor4d (1, 0, 0, 1);
                    glBegin (GL_LINE_STRIP);
                    channelHasTickSignal = true;
                    glVertex2d (0.0, lastHigh ? 1.0 : 0.0);
                }
                if (iSignal->second.clear) {
                    glVertex2d (iSignal->first - 1, lastHigh ? 1.0 : 0.0);
                    glVertex2d (iSignal->first - 1, 0.0);
                    lastHigh = false;
                } else {
                    glVertex2d (iSignal->first - 1, lastHigh ? 1.0 : 0.0);
                    glVertex2d (iSignal->first - 1, 1.0);
                    lastHigh = true;
                }
            }
            iSignal++;
        }
        if (channelHasTickSignal) {
            glVertex2d (stimulus->getDuration_s () / stimulus->sequence->tickInterval, lastHigh ? 1.0 : 0.0);
            glEnd ();
            cChannel++;
        } else if (signals)
            cChannel++;
        glPopMatrix ();
    }
    */
}

void StimulusRenderer::renderSpatialKernel (float min, float max, float width, float height)
{
    throw std::runtime_error (Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString ());
    /*
    if (kernelShader == nullptr) {
        glClear (GL_COLOR_BUFFER_BIT);
        return;
    }

    kernelShader->enable ();
    //float aspect = stimulus->sequence->getSpatialFilteredFieldWidth_um() / stimulus->sequence->getSpatialFilteredFieldHeight_um();
    kernelShader->bindUniformBool ("kernelGivenInFrequencyDomain", false);
    kernelShader->bindUniformFloat2 ("patternSizeOnRetina", width, height);

    for (auto& setting : stimulus->spatialFilter->shaderVariables) {
        kernelShader->bindUniformFloat (setting.first.c_str (), setting.second);
    }
    for (auto& setting : stimulus->spatialFilter->shaderColors) {
        kernelShader->bindUniformFloat3 (setting.first.c_str (), setting.second.x, setting.second.y, setting.second.z);
    }

    kernelShader->bindUniformFloat ("plotMin", min);
    kernelShader->bindUniformFloat ("plotMax", max);
    kernelShader->bindUniformFloat3 ("plotDarkColor", 0, 0, 0);
    kernelShader->bindUniformFloat3 ("plotBrightColor", 1, 0, 0);

    sequenceRenderer->getNothing ()->renderQuad ();
    kernelShader->disable ();
    */
}

void StimulusRenderer::renderSpatialProfile (float min, float max, float width, float height)
{
    throw std::runtime_error (Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString ());

    if (profileShader == nullptr) {
        //    glClear (GL_COLOR_BUFFER_BIT);
        return;
    }

    profileShader->enable ();
    profileShader->bindUniformFloat2 ("patternSizeOnRetina", width, height);
    //	profileShader->bindUniformFloat2("patternSizeOnRetina", stimulus->sequence->getSpatialFilteredFieldWidth_um(), stimulus->sequence->getSpatialFilteredFieldHeight_um());

    for (auto& setting : stimulus->spatialFilter->shaderVariables) {
        profileShader->bindUniformFloat (setting.first.c_str (), setting.second);
    }
    for (auto& setting : stimulus->spatialFilter->shaderColors) {
        profileShader->bindUniformFloat3 (setting.first.c_str (), setting.second.x, setting.second.y, setting.second.z);
    }

    //float aspect = stimulus->sequence->getSpatialFilteredFieldWidth_um() / stimulus->sequence->getSpatialFilteredFieldHeight_um();

    profileShader->bindUniformFloat ("plotMin", min);
    profileShader->bindUniformFloat ("plotMax", max);
    profileShader->bindUniformFloat3 ("plotDarkColor", 0, 0, 0);
    profileShader->bindUniformFloat3 ("plotBrightColor", 1, 0, 0);

    sequenceRenderer->getNothing ()->renderLineStrip (512);
    profileShader->disable ();
}

void StimulusRenderer::reset ()
{
    iFrame = 1;
    iTick  = 1;
    for (auto& p : passRenderers)
        p->reset ();
}

void StimulusRenderer::renderTemporalKernel ()
{
    throw std::runtime_error (Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString ());
    /*
    glColor3d (1, 0, 0);
    glLineWidth (2);
    glPushMatrix ();
    glLoadIdentity ();
    glScaled (-0.9, 0.9, 1);
    glTranslated (-1, 0, 0);
    glScaled (2.0 / stimulus->temporalMemoryLength, 2 / (stimulus->temporalWeightMax - stimulus->temporalWeightMin), 1.0);
    glTranslated (0, -(stimulus->temporalWeightMax + stimulus->temporalWeightMin) * 0.5, 0);
    glEnable (GL_LINE_STIPPLE);
    glLineStipple (2, 0x5555);
    glBegin (GL_LINES);
    glVertex2d (-100, 0);
    glVertex2d (100, 0);
    glVertex2d (0, 100);
    glVertex2d (0, -100);
    glEnd ();

    glDisable (GL_LINE_STIPPLE);

    if (stimulus->temporalProcessingStateCount > 0) {
        glColor3d (1, 0.0, 0);
        glBegin (GL_LINE_STRIP);
        for (unsigned int i = 0; i < stimulus->temporalMemoryLength; i++) {
            glVertex2d (stimulus->temporalMemoryLength - i, stimulus->temporalWeights[stimulus->temporalMemoryLength - i - 1]);
            glVertex2d (stimulus->temporalMemoryLength - 1 - i, stimulus->temporalWeights[stimulus->temporalMemoryLength - i - 1]);
        }
        glEnd ();
    } else {
        temporalProfileShader->enable ();
        temporalProfileShader->bindUniformInt ("memoryLength", stimulus->temporalMemoryLength);
        temporalProfileShader->bindUniformInt ("queueLength", sequenceRenderer->getSequence ()->getMaxMemoryLength ());
        temporalProfileShader->bindUniformTexture1D ("gamma", gammaTexture->getTextureHandle (), 1);

        glColor3d (1, 0, 0);
        sequenceRenderer->getNothing ()->renderLineStrip (stimulus->temporalMemoryLength * 2);
        temporalProfileShader->disable ();
    }

    glPopMatrix ();
    */
}

void StimulusRenderer::skipFrames (uint nFramesToSkip)
{
    iFrame += nFramesToSkip;
}


bool                              StimulusRenderer::hasSpatialFilter () const { return spatialFilterRenderer != nullptr; }
std::shared_ptr<SequenceRenderer> StimulusRenderer::getSequenceRenderer () const { return sequenceRenderer; }