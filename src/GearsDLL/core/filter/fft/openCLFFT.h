#if 0

#ifndef OPENCLFFT_H
#define OPENCLFFT_H

#include "FFT.h"
#include "openCLCore.h"
#include <cstring>
#include <functional>
#include <windows.h>

class FFTTest;

class OPENCLFFT : public FFT
{
public:
    OPENCLFFT (unsigned int width, unsigned int height, FFTChannelMode channelMode = FFTChannelMode::Monochrome, unsigned int input_tex = 0);
    ~OPENCLFFT ();

    virtual void do_fft () override;
    virtual void do_inverse_fft ();
    virtual unsigned int get_fullTex () const override;
    void get_channels (cl_mem& r) const;
    void take_channels (cl_mem& r, cl_mem& g, cl_mem& b);
    virtual unsigned int take_fullTex_ownership () override;
    virtual void redraw_input () override;
    virtual bool storeFrequencyInTexture () const override { return false; }
    void finish () { if (queue) clFinish (queue); }
    void finishConv ();
    cl_command_queue getQueue () const { return queue; }
    bool HasImageObject () { return hasImageObject; }
    static void staticInit ();

protected_test:
    unsigned int fullTex;
    unsigned short has_input_tex;
    bool transformed;
    bool ownsChannels;
    bool hasImageObject = false;
    FFTChannelMode channelMode;

    cl_mem clImgr = nullptr;
    cl_mem clImgg = nullptr;
    cl_mem clImgb = nullptr;
    cl_mem clImgFull = nullptr;

    cl_context ctx;
    cl_device_id device;
    cl_command_queue queue = 0;

    clfftPlanHandle planHandleFFT;
    clfftPlanHandle planHandleIFFT;

    size_t full_img_size;
    size_t channel_img_size;

    //Enqueue the kernel for execution
    cl_uint work_dim = 1;
    size_t global_work_offset = 0;
    size_t global_work_size[1];
    size_t local_work_size[1];

    size_t origin[3];
    size_t region[3];

    float* img = nullptr;

    void init_framebuffer ();
    void bakePlans ();

    void initCl ();
    void separateChannels (FFTChannelMode channelMode);
    void combineChannels ();
};

#endif // OPENCLFFT_H

#endif