#pragma once

#include "gpu/Framebuffer.hpp"
#include "gpu/Pointgrid.hpp"
#include "gpu/Quad.hpp"
#include "gpu/RandomSequenceBuffer.hpp"
#include "gpu/Shader.hpp"
#include "gpu/StimulusGrid.hpp"
#include "gpu/Texture.hpp"
#include "gpu/TextureQueue.hpp"

#include "core/Sequence.h"
#include "core/filter/KernelManager.h"
#include "core/filter/SpatialFilter.h"
#include "fft/FFT.h"
#include <functional>
#include <map>
#include <string>

class SequenceRenderer;

//! Represents the currently active sequence. Manages resources for GPU computation.
class SpatialFilterRenderer {
protected:
    std::shared_ptr<SequenceRenderer> sequenceRenderer;
    unsigned int                      spatialKernelId = 0;
    Shader*                           spatialDomainConvolutionShader;
    Shader*                           copyShader;

    std::function<void (int)> renderStim;
    FFTChannelMode            channelMode;
    std::function<void ()>    renderQuad;

    SpatialFilter::P spatialFilter;
    KernelManager::P kernelManager;
    ShaderManager::P shaderManager;
    SpatialFilterRenderer (std::shared_ptr<SequenceRenderer> sequenceRenderer, ShaderManager::P shaderManager, KernelManager::P _kernelManager, SpatialFilter::P _spatialFilter);

public:
    void renderFrame (std::function<void (int)> renderStimulus);

    virtual void initFirstFrames (std::function<void (int)> stim) {}
    virtual void prepareNext () {};

protected:
    virtual void fftConvolution () = 0;
    virtual void bindTexture ()    = 0;
    void         normalConvolution ();
};