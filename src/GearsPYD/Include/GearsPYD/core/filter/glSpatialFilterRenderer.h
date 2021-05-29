#pragma once

#if 0
#include "SpatialFilterRenderer.h"

class GLSpatialFilterRenderer : public SpatialFilterRenderer {
    GLFFT   fft_rg;
    GLFFT   fft_ba;
    GLFFT   ifft_rg;
    GLFFT   ifft_ba;
    Shader* convolutionShader;

protected:
    void fftConvolution () override;
    void bindTexture () override;

public:
    GLSpatialFilterRenderer (std::shared_ptr<SequenceRenderer> sequenceRenderer, ShaderManager::P shaderManager, KernelManager::P _kernelManager, SpatialFilter::P _spatialFilter, unsigned int width, unsigned int height);
};

#endif