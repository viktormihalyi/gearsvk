#include <iostream>
#include "openCLFFT.h"
#include <ctime>
#include <chrono>
#include <algorithm>
#include <GL/glew.h>

const char* separateChannelsProgram = R"(
		__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
		__kernel void separateChannels(
		  __read_only image2d_t fullImg,
		  const int w,
		  __global float* imgrgb)
		  {
			int i = get_global_id( 0 );
			int channelIndex = (i/w)*(w+2)+(i%w);
			imgrgb[channelIndex] = read_imagef( fullImg, sampler, (int2)(i / w, i % w) ).x;
			imgrgb[channelIndex + (w+2)*w] = read_imagef( fullImg, sampler, (int2)(i / w, i % w) ).y;
			imgrgb[channelIndex + (w+2)*w*2] = read_imagef( fullImg, sampler, (int2)(i / w, i % w) ).z;
		  })";

const char* separateChannelsMonoProgram = R"(
		__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE	| CLK_FILTER_NEAREST;
		__kernel void separateChannelsMono(
		  __read_only image2d_t fullImg,
		  const int w,
		  __global float* imgr)
		  {
			int i = get_global_id( 0 );
			int channelIndex = (i/w)*(w+2)+(i%w);
			imgr[channelIndex] = read_imagef( fullImg, sampler, (int2)(i / w, i % w) ).x;
		  })";

const char* combineChannelsProgram = R"(
		__kernel void combineChannels(
		  __write_only image2d_t fullImg,
		  const int w,
		  __global float* imgrgb)
		  {
			int i = get_global_id( 0 );
			int index = i + (i/w)*2;
			write_imagef( fullImg, (int2)(i/w, i%w), (float4)(imgrgb[index], imgrgb[index+(w+2)*w], imgrgb[index+(w+2)*w*2], 1.0f) );
		  })";

const char* combineChannelsMonoProgram = R"(
		__kernel void combineChannelsMono(
		  __write_only image2d_t fullImg,
		  const int w,
		  __global float* imgr)
		  {
			int i = get_global_id( 0 );
			int index = i + (i/w)*2;
			write_imagef( fullImg, (int2)(i/w, i%w), (float4)(imgr[index], imgr[index], imgr[index], 1.0f) );
		  })";

OPENCLFFT::OPENCLFFT (unsigned int width, unsigned int height, FFTChannelMode channelMode, unsigned int input_tex) :
    FFT (width, height), fullTex (input_tex),
    full_img_size (size[0] * size[1] * 4), channel_img_size (size[1] * (size[0] + 2)), transformed (false), ownsChannels (true), channelMode (channelMode)
{
    ownsTex = false;
    has_input_tex = glIsTexture (input_tex);
    init_framebuffer ();
    initCl ();

    for (size_t i = 0; i < 3; i++) {
        origin[i] = 0;
        region[i] = 1;
    }

    global_work_size[0] = full_img_size / 4;

    local_work_size[0] = (64 < global_work_size[0] ? 64 : 4);

    region[0] = size[0];
    region[1] = size[1];

    bakePlans ();
}

OPENCLFFT::~OPENCLFFT ()
{
    clReleaseMemObject (clImgFull);
    if (ownsTex)
        glDeleteTextures (1, &fullTex);

    /* Release the plan. */
    clfftDestroyPlan (&planHandleFFT);
    clfftDestroyPlan (&planHandleIFFT);

    if (ownsChannels) {
        if (clImgr)
            clReleaseMemObject (clImgr);
        if (clImgg)
            clReleaseMemObject (clImgg);
        if (clImgb)
            clReleaseMemObject (clImgb);
    }
}

void OPENCLFFT::init_framebuffer ()
{
    if (!has_input_tex) {
        glGenTextures (1, &fullTex);
        glBindTexture (GL_TEXTURE_RECTANGLE_ARB, fullTex);
        glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB, size[0], size[1], 0, GL_RGBA, GL_FLOAT, 0);
    }
    glGenFramebuffersEXT (1, &fbo);
    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, fbo);
    glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, fullTex, 0);
    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
}

void OPENCLFFT::bakePlans ()
{
    using OpenCLHelper::clPrintError;

    cl_int err;
    clfftDim dim = CLFFT_2D;

    /* Create a default plan for a complex FFT. */
    err = clfftCreateDefaultPlan (&planHandleFFT, ctx, dim, size);
    clPrintError (err);
    err = clfftCreateDefaultPlan (&planHandleIFFT, ctx, dim, size);
    clPrintError (err);

    /* Set plan parameters. */
    err = clfftSetPlanPrecision (planHandleFFT, CLFFT_SINGLE);
    clPrintError (err);
    err = clfftSetLayout (planHandleFFT, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);
    clPrintError (err);
    size_t strides[2] = { 1, size[0] + 2 };
    clfftSetPlanInStride (planHandleFFT, dim, strides);
    strides[1] = (size[0] / 2 + 1);
    clfftSetPlanOutStride (planHandleFFT, dim, strides);
    err = clfftSetResultLocation (planHandleFFT, CLFFT_INPLACE);
    clPrintError (err);

    err = clfftSetPlanPrecision (planHandleIFFT, CLFFT_SINGLE);
    clPrintError (err);
    err = clfftSetLayout (planHandleIFFT, CLFFT_HERMITIAN_INTERLEAVED, CLFFT_REAL);
    clPrintError (err);
    strides[1] = size[0] / 2 + 1;
    clfftSetPlanInStride (planHandleIFFT, dim, strides);
    strides[1] = size[0] + 2;
    clfftSetPlanOutStride (planHandleIFFT, dim, strides);
    err = clfftSetResultLocation (planHandleIFFT, CLFFT_INPLACE);
    clPrintError (err);

    if (channelMode == FFTChannelMode::Multichrome) {
        clfftSetPlanBatchSize (planHandleFFT, 3);
        clfftSetPlanDistance (planHandleFFT, channel_img_size, channel_img_size / 2);

        clfftSetPlanBatchSize (planHandleIFFT, 3);
        clfftSetPlanDistance (planHandleIFFT, channel_img_size / 2, channel_img_size);
    }

    /* Bake the plan. */
    err = clfftBakePlan (planHandleFFT, 1, &queue, NULL, NULL);
    clPrintError (err);
    err = clfftBakePlan (planHandleIFFT, 1, &queue, NULL, NULL);
    clPrintError (err);

}

void OPENCLFFT::staticInit ()
{
    OpenCLCore::Get ()->RegistKernel ("separateChannels", separateChannelsProgram, 0, true);
    OpenCLCore::Get ()->RegistKernel ("combineChannels", combineChannelsProgram, 0, true);
    OpenCLCore::Get ()->RegistKernel ("separateChannelsMono", separateChannelsMonoProgram, 0, true);
    OpenCLCore::Get ()->RegistKernel ("combineChannelsMono", combineChannelsMonoProgram, 0, true);
}

void OPENCLFFT::initCl ()
{
    using OpenCLHelper::clPrintError;

    cl_int err;

    ctx = OpenCLCore::Get ()->ctx;
    device = OpenCLCore::Get ()->device;

    queue = OpenCLCore::Get ()->createCommandQueue ();

    clImgFull = clCreateFromGLTexture (ctx, CL_MEM_READ_WRITE, GL_TEXTURE_RECTANGLE_ARB, 0, fullTex, &err);
    clPrintError (err);

    clImgr = clCreateBuffer (ctx, CL_MEM_READ_ONLY, (channelMode == FFTChannelMode::Monochrome ? 1 : 3) * channel_img_size * sizeof (float), NULL, &err);
    clPrintError (err);
    clImgg = clCreateBuffer (ctx, CL_MEM_READ_ONLY, channel_img_size * sizeof (float), NULL, &err);
    clPrintError (err);
    clImgb = clCreateBuffer (ctx, CL_MEM_READ_ONLY, channel_img_size * sizeof (float), NULL, &err);
    clPrintError (err);
}

void OPENCLFFT::separateChannels (FFTChannelMode channelMode)
{

    using OpenCLHelper::clPrintError;

    cl_int err;
    cl_kernel separatorKernel = nullptr;
    switch (channelMode) {
        case FFTChannelMode::Monochrome:
            separatorKernel = OpenCLCore::Get ()->GetKernel ("separateChannelsMono");
            break;
        case FFTChannelMode::Multichrome:
            separatorKernel = OpenCLCore::Get ()->GetKernel ("separateChannels");
            break;
        default:
            return;
    }
    if (!separatorKernel)
        return;

    err = clSetKernelArg (separatorKernel, 1, sizeof (int), (void*)&size[0]);
    clPrintError (err);

    err = clSetKernelArg (separatorKernel, 2, sizeof (cl_mem), &clImgr);
    clPrintError (err);

    clEnqueueAcquireGLObjects (queue, 1, &clImgFull, 0, 0, NULL);
    hasImageObject = true;

    //Kernels argument settings
    err = clSetKernelArg (separatorKernel, 0, sizeof (cl_mem), &clImgFull);
    clPrintError (err);
    err = clEnqueueNDRangeKernel (queue, separatorKernel, work_dim, &global_work_offset, global_work_size, local_work_size, 0, NULL, NULL);
    clPrintError (err);
}

void OPENCLFFT::combineChannels ()
{
    using OpenCLHelper::clPrintError;

    cl_int err = 0;

    auto start = std::chrono::system_clock::now ();

    cl_kernel combinatorKernel = nullptr;
    switch (channelMode) {
        case FFTChannelMode::Monochrome:
            combinatorKernel = OpenCLCore::Get ()->GetKernel ("combineChannelsMono");
            break;
        case FFTChannelMode::Multichrome:
            combinatorKernel = OpenCLCore::Get ()->GetKernel ("combineChannels");
            break;
    }
    if (!combinatorKernel)
        return;

    //Kenels argument settings
    err = clSetKernelArg (combinatorKernel, 1, sizeof (int), (void*)&size[0]);
    clPrintError (err);

    err = clSetKernelArg (combinatorKernel, 2, sizeof (cl_mem), &clImgr);
    clPrintError (err);

    err = clSetKernelArg (combinatorKernel, 0, sizeof (cl_mem), &clImgFull);
    clPrintError (err);
    err = clEnqueueNDRangeKernel (queue, combinatorKernel, work_dim, &global_work_offset, global_work_size, local_work_size, 0, NULL, NULL);
    clPrintError (err);

    auto clEnd = std::chrono::system_clock::now ();

    auto end = std::chrono::system_clock::now ();
    std::chrono::duration<double> elapsedSeconds = clEnd - start;
}

void OPENCLFFT::do_inverse_fft ()
{
    if (!transformed)
        return;
    auto start = std::chrono::system_clock::now ();

    clfftEnqueueTransform (planHandleIFFT, CLFFT_BACKWARD, 1, &queue, 0, NULL, NULL, &clImgr, NULL, NULL);
    transformed = false;

    /* Combine channels and write it two texture */
    auto enqueueEnd = std::chrono::system_clock::now ();
    combineChannels ();

    auto end = std::chrono::system_clock::now ();
    std::chrono::duration<double> elapsedSeconds = enqueueEnd - start;
}

void OPENCLFFT::finishConv ()
{
    clFinish (queue);
    clEnqueueReleaseGLObjects (queue, 1, &clImgFull, 0, 0, NULL);
    hasImageObject = false;
}

void OPENCLFFT::do_fft ()
{
    if (!fullTex || transformed)
        return;
    if (!redrawn)
        redraw_input ();

    auto start = std::chrono::system_clock::now ();
    separateChannels (channelMode);

    auto end = std::chrono::system_clock::now ();
    std::chrono::duration<double> elapsedSeconds = end - start;
    std::cout << "\tLength of separate channels " << elapsedSeconds.count () * 1000 << "ms." << std::endl;

    start = std::chrono::system_clock::now ();
    clfftEnqueueTransform (planHandleFFT, CLFFT_FORWARD, 1, &queue, 0, NULL, NULL, &clImgr, NULL, NULL);
    end = std::chrono::system_clock::now ();
    elapsedSeconds = end - start;
    std::cout << "\tLength of enqueue transform " << elapsedSeconds.count () * 1000 << "ms." << std::endl;

    transformed = true;
}

void OPENCLFFT::get_channels (cl_mem& r) const
{
    r = clImgr;
}

unsigned int OPENCLFFT::get_fullTex () const
{
    return fullTex;
}

unsigned int OPENCLFFT::take_fullTex_ownership ()
{
    ownsTex = false;
    return fullTex;
}

void OPENCLFFT::take_channels (cl_mem& r, cl_mem& g, cl_mem& b)
{
    r = clImgr;
    g = clImgg;
    b = clImgb;
    ownsChannels = false;
}

void OPENCLFFT::redraw_input ()
{
    redrawn = true;
    if (draw_input) {
        int vp[4];
        glGetIntegerv (GL_VIEWPORT, vp);
        glViewport (0, 0, size[0] + 1, size[1] + 1);

        glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, fbo);
        glDrawBuffer (GL_COLOR_ATTACHMENT0_EXT);
        draw_input (1);
        glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);

        glViewport (vp[0], vp[1], vp[2], vp[3]);
        redrawn = true;
    }
}