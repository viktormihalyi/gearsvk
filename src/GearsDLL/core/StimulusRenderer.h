#pragma once

#include "gpu/Framebuffer.hpp"
#include "gpu/Pointgrid.hpp"
#include "gpu/Quad.hpp"
#include "gpu/RandomSequenceBuffer.hpp"
#include "gpu/Shader.hpp"
#include "gpu/StimulusGrid.hpp"
#include "gpu/Texture.hpp"
#include "gpu/TextureQueue.hpp"

#include "core/PassRenderer.h"
#include "core/ShaderManager.h"
#include "core/TextureManager.h"
#include "filter/KernelManager.h"
#include "filter/SpatialFilterRenderer.h"
#include <map>
#include <string>

class SequenceRenderer;
class PassRenderer;

//! Represents the currently active sequence. Manages resources for GPU computation.
class StimulusRenderer {
    friend class PassRenderer;
    std::shared_ptr<SequenceRenderer> sequenceRenderer;

    unsigned int                           iFrame;
    unsigned int                           iTick;
    Stimulus::CP                           stimulus;
    std::unique_ptr<SpatialFilterRenderer> spatialFilterRenderer;

    Shader* randomGeneratorShader;
    Shader* particleShader;
    Shader* stimulusGeneratorShader;
    Shader* temporalFilteringShader;
    Shader* dynamicToneShader;
    Shader* timelineShader;
    Shader* spikeShader;
    Shader* kernelShader;
    Shader* profileShader;
    Shader* temporalProfileShader;

    Texture1D* gammaTexture;
    Texture2D* measuredHistogramTexture;

    std::vector<PassRenderer::P> passRenderers;

    StimulusRenderer (std::shared_ptr<SequenceRenderer> sequenceRenderer, Stimulus::CP stimulus, ShaderManager::P shaderManager, TextureManager::P textureManager, KernelManager::P kernelManager);

public:
    GEARS_SHARED_CREATE_WITH_GETSHAREDPTR (StimulusRenderer);
    ~StimulusRenderer ();
    void apply (ShaderManager::P shaderManager, TextureManager::P textureManager);

    void preRender ();
    void renderStimulus (GLuint defaultFrameBuffer, int skippedFrames);
    void renderSample (uint sFrame, int left, int top, int width, int height);
    void renderTimeline (bool* signals, uint startFrame, uint frameCount);
    void renderSpatialKernel (float min, float max, float width, float height);
    void renderSpatialProfile (float min, float max, float width, float height);
    void renderTemporalKernel ();

    unsigned int getCurrentFrame () { return iFrame; }
    unsigned int tick () { return iTick++; }

    Stimulus::CP getStimulus () const { return stimulus; }

    void reset ();
    void skipFrames (uint nFramesToSkip);

    bool                              hasSpatialFilter () const { return spatialFilterRenderer != nullptr; }
    std::shared_ptr<SequenceRenderer> getSequenceRenderer () const { return sequenceRenderer; }
};