#pragma once

#include "stdafx.h"
#include <memory>

#include <functional>
#include <map>
#include <string>

class SequenceRenderer;
class SpatialFilter;
class KernelManager;
class ShaderManager;
class SpatialFilter;
enum class FFTChannelMode : uint8_t;

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

    std::shared_ptr<SpatialFilter> spatialFilter;
    std::shared_ptr<KernelManager> kernelManager;
    std::shared_ptr<ShaderManager> shaderManager;
    SpatialFilterRenderer (std::shared_ptr<SequenceRenderer> sequenceRenderer, std::shared_ptr<ShaderManager> shaderManager, std::shared_ptr<KernelManager> _kernelManager, std::shared_ptr<SpatialFilter> _spatialFilter);

public:
    void renderFrame (std::function<void (int)> renderStimulus);

    virtual void initFirstFrames (std::function<void (int)> stim) {}
    virtual void prepareNext () {};

protected:
    virtual void fftConvolution () = 0;
    virtual void bindTexture ()    = 0;
    void         normalConvolution ();
};