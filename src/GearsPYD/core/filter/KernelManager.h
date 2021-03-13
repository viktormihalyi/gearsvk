#pragma once

#include <map>
#include <string>

#include "stdafx.h"
#include <memory>

class SequenceRenderer;
class FFT;
class FramebufferGL;
class Shader;
class SpatialFilter;

class KernelManager {
    std::shared_ptr<SequenceRenderer> sequenceRenderer;
    std::shared_ptr<ShaderManager>    shaderManager;

    KernelManager (std::shared_ptr<SequenceRenderer> sequenceRenderer, std::shared_ptr<ShaderManager> shaderManager);

public:
    GEARS_SHARED_CREATE (KernelManager);

    struct Kernel {
        FFT*           fft;
        FramebufferGL* buff;
        Shader*        kernelShader;
    };

    using KernelMap = std::map<std::string, Kernel>;

    unsigned int getKernel (std::shared_ptr<SpatialFilter const> spatialFilter);
    //bool         getKernelChannels (std::shared_ptr<SpatialFilter const> spatialFilter, cl_mem& r);
    unsigned int update (std::shared_ptr<SpatialFilter const> spatialFilter);

    void clear ();

private:
    KernelMap kernels;
};
