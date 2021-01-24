#pragma once

#include <map>
#include <string>

#include "Ptr.hpp"
#include "stdafx.h"

class SequenceRenderer;
class FFT;
class FramebufferGL;
class Shader;
class SpatialFilter;

class KernelManager {
    Ptr<SequenceRenderer> sequenceRenderer;
    Ptr<ShaderManager>    shaderManager;

    KernelManager (Ptr<SequenceRenderer> sequenceRenderer, Ptr<ShaderManager> shaderManager);

public:
    GEARS_SHARED_CREATE (KernelManager);

    struct Kernel {
        FFT*           fft;
        FramebufferGL* buff;
        Shader*        kernelShader;
    };

    using KernelMap = std::map<std::string, Kernel>;

    unsigned int getKernel (PtrC<SpatialFilter> spatialFilter);
    //bool         getKernelChannels (PtrC<SpatialFilter> spatialFilter, cl_mem& r);
    unsigned int update (PtrC<SpatialFilter> spatialFilter);

    void clear ();

private:
    KernelMap kernels;
};
