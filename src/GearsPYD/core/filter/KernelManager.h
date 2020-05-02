#pragma once

#include "gpu/Framebuffer.hpp"
#include "gpu/Pointgrid.hpp"
#include "gpu/Quad.hpp"
#include "gpu/RandomSequenceBuffer.hpp"
#include "gpu/Shader.hpp"
#include "gpu/StimulusGrid.hpp"
#include "gpu/Texture.hpp"
#include "gpu/TextureQueue.hpp"

#include "core/ShaderManager.h"
#include "core/filter/SpatialFilter.h"
#include "core/filter/fft/glFFT.h"
#include "core/filter/fft/openCLFFT.h"

#include <map>
#include <string>

class SequenceRenderer;

class KernelManager {
    std::shared_ptr<SequenceRenderer> sequenceRenderer;
    ShaderManager::P                  shaderManager;

    KernelManager (std::shared_ptr<SequenceRenderer> sequenceRenderer, ShaderManager::P shaderManager);

public:
    GEARS_SHARED_CREATE (KernelManager);

    struct Kernel {
        FFT*           fft;
        FramebufferGL* buff;
        Shader*        kernelShader;
    };

    using KernelMap = std::map<std::string, Kernel>;

    unsigned int getKernel (SpatialFilter::CP spatialFilter);
    //bool         getKernelChannels (SpatialFilter::CP spatialFilter, cl_mem& r);
    unsigned int update (SpatialFilter::CP spatialFilter);

    void clear ();

private:
    KernelMap kernels;
};
