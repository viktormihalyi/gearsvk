#pragma once

#include <map>
#include <string>

#include "OpenGLProxy.hpp"
#include "Ptr.hpp"
#include "stdafx.h"

class ShaderManager;
class TextureManager;
class SequenceRenderer;
class KernelManager;
class PassRenderer;
class Stimulus;
class Shader;
class Texture1D;
class Texture2D;
class SpatialFilterRenderer;

//! Represents the currently active sequence. Manages resources for GPU computation.
class StimulusRenderer {
    friend class PassRenderer;
    Ptr<SequenceRenderer> sequenceRenderer;

    unsigned int             iFrame;
    unsigned int             iTick;
    PtrC<Stimulus>           stimulus;
    U<SpatialFilterRenderer> spatialFilterRenderer;

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

    std::vector<Ptr<PassRenderer>> passRenderers;

    StimulusRenderer (Ptr<SequenceRenderer> sequenceRenderer, PtrC<Stimulus> stimulus, Ptr<ShaderManager> shaderManager, Ptr<TextureManager> textureManager, Ptr<KernelManager> kernelManager);

public:
    GEARS_SHARED_CREATE_WITH_GETSHAREDPTR (StimulusRenderer);
    ~StimulusRenderer ();
    void apply (Ptr<ShaderManager> shaderManager, Ptr<TextureManager> textureManager);

    void preRender ();
    void renderStimulus (GLuint defaultFrameBuffer, int skippedFrames);
    void renderSample (uint sFrame, int left, int top, int width, int height);
    void renderTimeline (bool* signals, uint startFrame, uint frameCount);
    void renderSpatialKernel (float min, float max, float width, float height);
    void renderSpatialProfile (float min, float max, float width, float height);
    void renderTemporalKernel ();

    unsigned int getCurrentFrame () { return iFrame; }
    unsigned int tick () { return iTick++; }

    PtrC<Stimulus> getStimulus () const { return stimulus; }

    void reset ();
    void skipFrames (uint nFramesToSkip);

    bool                  hasSpatialFilter () const;
    Ptr<SequenceRenderer> getSequenceRenderer () const;
};