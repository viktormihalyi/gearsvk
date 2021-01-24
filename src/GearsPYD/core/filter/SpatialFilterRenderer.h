#pragma once

#include "Ptr.hpp"
#include "stdafx.h"

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
    Ptr<SequenceRenderer> sequenceRenderer;
    unsigned int          spatialKernelId = 0;
    Shader*               spatialDomainConvolutionShader;
    Shader*               copyShader;

    std::function<void (int)> renderStim;
    FFTChannelMode            channelMode;
    std::function<void ()>    renderQuad;

    Ptr<SpatialFilter> spatialFilter;
    Ptr<KernelManager> kernelManager;
    Ptr<ShaderManager> shaderManager;
    SpatialFilterRenderer (Ptr<SequenceRenderer> sequenceRenderer, Ptr<ShaderManager> shaderManager, Ptr<KernelManager> _kernelManager, Ptr<SpatialFilter> _spatialFilter);

public:
    void renderFrame (std::function<void (int)> renderStimulus);

    virtual void initFirstFrames (std::function<void (int)> stim) {}
    virtual void prepareNext () {};

protected:
    virtual void fftConvolution () = 0;
    virtual void bindTexture ()    = 0;
    void         normalConvolution ();
};