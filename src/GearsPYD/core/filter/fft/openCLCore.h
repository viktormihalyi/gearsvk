#pragma once
#if 0

#include <CL/cl_gl.h>
#include <clFFT.h>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <windows.h>

struct OpenCLKernelEntry
{
    const char* source;
    size_t sourceSize;
    cl_kernel kernel = nullptr;
};

namespace ImageHelper
{
void printImg (const float* img, unsigned w, unsigned h, const char* name = "FFT", unsigned channels = 4, bool complex = false, unsigned pad = 0, unsigned offsetW = 0, unsigned offsetH = 0);
void printImgChannel (const float* img, unsigned w, unsigned h, unsigned channels, unsigned channel, const char* name = "FFT", bool complex = false, unsigned pad = 0, unsigned offsetW = 0, unsigned offsetH = 0);
void printImgStream (std::ostream& st, const float* img, unsigned w, unsigned h, const char* name = "FFT", unsigned channels = 4, bool complex = false, unsigned pad = 0, unsigned offsetW = 0, unsigned offsetH = 0);
}

namespace OpenCLHelper
{
void clPrintError (cl_int& errorCode);
}

class OpenCLCore
{
    OpenCLCore ();
    static OpenCLCore* _instance;

    std::map<std::string, OpenCLKernelEntry> kernels;

public:
    cl_context ctx = nullptr;
    cl_device_id device = nullptr;
    cl_command_queue queue = 0;

    static OpenCLCore* Get ();
    void getClData (cl_mem mem, float* data, unsigned size);
    static cl_kernel GetKernel (std::string name);
    static void RegistKernel (std::string name, const char* source, size_t sourceSize = 0, bool compile = false);
    static cl_kernel CompileKernel (const char* name, const char* source, size_t sourceSize = 0);
    static void MultiplyMonoFFT (cl_command_queue q, cl_mem lhs, cl_mem rhs, size_t* global_work_size, size_t* local_work_size = nullptr);
    static void MultiplyFFT (cl_command_queue q, cl_mem lhs, cl_mem rhs, size_t* global_work_size, size_t* local_work_size = nullptr);
    void finish () { if (queue) clFinish (queue); }
    cl_command_queue createCommandQueue ();
    static void Destroy ();
};
#endif