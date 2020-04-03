#pragma once

#if 0
#include "SpatialFilterRenderer.h"


class CLSpatialFilterRenderer : public SpatialFilterRenderer {
    OPENCLFFT  fft;
    OPENCLFFT  fft_prepare;
    OPENCLFFT* current_fft = nullptr;
    OPENCLFFT* other_fft   = nullptr;
    unsigned   width;
    unsigned   height;
    bool       firstFrame = true;

protected:
    void fftConvolution () override;
    void bindTexture () override;
    void multiply (OPENCLFFT* fft);

public:
    CLSpatialFilterRenderer (std::shared_ptr<SequenceRenderer> sequenceRenderer, ShaderManager::P shaderManager, KernelManager::P _kernelManager, SpatialFilter::P _spatialFilter, unsigned int width, unsigned int height, FFTChannelMode channelMode);

    void initFirstFrames (std::function<void (int)> stim) override;
    void prepareNext () override;
};

#endif