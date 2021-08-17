#include "ResourceLimits.hpp"


namespace GVK {

TBuiltInResource GetDefaultResourceLimits ()
{
    static_assert (sizeof (TBuiltInResource) == 380);

    TBuiltInResource result = {};

    result.maxLights                                 = 32;
    result.maxClipPlanes                             = 6;
    result.maxTextureUnits                           = 32;
    result.maxTextureCoords                          = 32;
    result.maxVertexAttribs                          = 64;
    result.maxVertexUniformComponents                = 4096;
    result.maxVaryingFloats                          = 64;
    result.maxVertexTextureImageUnits                = 32;
    result.maxCombinedTextureImageUnits              = 80;
    result.maxTextureImageUnits                      = 32;
    result.maxFragmentUniformComponents              = 4096;
    result.maxDrawBuffers                            = 32;
    result.maxVertexUniformVectors                   = 128;
    result.maxVaryingVectors                         = 8;
    result.maxFragmentUniformVectors                 = 16;
    result.maxVertexOutputVectors                    = 16;
    result.maxFragmentInputVectors                   = 15;
    result.minProgramTexelOffset                     = -8;
    result.maxProgramTexelOffset                     = 7;
    result.maxClipDistances                          = 8;
    result.maxComputeWorkGroupCountX                 = 65535;
    result.maxComputeWorkGroupCountY                 = 65535;
    result.maxComputeWorkGroupCountZ                 = 65535;
    result.maxComputeWorkGroupSizeX                  = 1024;
    result.maxComputeWorkGroupSizeY                  = 1024;
    result.maxComputeWorkGroupSizeZ                  = 64;
    result.maxComputeUniformComponents               = 1024;
    result.maxComputeTextureImageUnits               = 16;
    result.maxComputeImageUniforms                   = 8;
    result.maxComputeAtomicCounters                  = 8;
    result.maxComputeAtomicCounterBuffers            = 1;
    result.maxVaryingComponents                      = 60;
    result.maxVertexOutputComponents                 = 64;
    result.maxGeometryInputComponents                = 64;
    result.maxGeometryOutputComponents               = 128;
    result.maxFragmentInputComponents                = 128;
    result.maxImageUnits                             = 8;
    result.maxCombinedImageUnitsAndFragmentOutputs   = 8;
    result.maxCombinedShaderOutputResources          = 8;
    result.maxImageSamples                           = 0;
    result.maxVertexImageUniforms                    = 0;
    result.maxTessControlImageUniforms               = 0;
    result.maxTessEvaluationImageUniforms            = 0;
    result.maxGeometryImageUniforms                  = 0;
    result.maxFragmentImageUniforms                  = 8;
    result.maxCombinedImageUniforms                  = 8;
    result.maxGeometryTextureImageUnits              = 16;
    result.maxGeometryOutputVertices                 = 256;
    result.maxGeometryTotalOutputComponents          = 1024;
    result.maxGeometryUniformComponents              = 1024;
    result.maxGeometryVaryingComponents              = 64;
    result.maxTessControlInputComponents             = 128;
    result.maxTessControlOutputComponents            = 128;
    result.maxTessControlTextureImageUnits           = 16;
    result.maxTessControlUniformComponents           = 1024;
    result.maxTessControlTotalOutputComponents       = 4096;
    result.maxTessEvaluationInputComponents          = 128;
    result.maxTessEvaluationOutputComponents         = 128;
    result.maxTessEvaluationTextureImageUnits        = 16;
    result.maxTessEvaluationUniformComponents        = 1024;
    result.maxTessPatchComponents                    = 120;
    result.maxPatchVertices                          = 32;
    result.maxTessGenLevel                           = 64;
    result.maxViewports                              = 16;
    result.maxVertexAtomicCounters                   = 0;
    result.maxTessControlAtomicCounters              = 0;
    result.maxTessEvaluationAtomicCounters           = 0;
    result.maxGeometryAtomicCounters                 = 0;
    result.maxFragmentAtomicCounters                 = 8;
    result.maxCombinedAtomicCounters                 = 8;
    result.maxAtomicCounterBindings                  = 1;
    result.maxVertexAtomicCounterBuffers             = 0;
    result.maxTessControlAtomicCounterBuffers        = 0;
    result.maxTessEvaluationAtomicCounterBuffers     = 0;
    result.maxGeometryAtomicCounterBuffers           = 0;
    result.maxFragmentAtomicCounterBuffers           = 1;
    result.maxCombinedAtomicCounterBuffers           = 1;
    result.maxAtomicCounterBufferSize                = 16384;
    result.maxTransformFeedbackBuffers               = 4;
    result.maxTransformFeedbackInterleavedComponents = 64;
    result.maxCullDistances                          = 8;
    result.maxCombinedClipAndCullDistances           = 8;
    result.maxSamples                                = 4;
    result.maxMeshOutputVerticesNV                   = 256;
    result.maxMeshOutputPrimitivesNV                 = 512;
    result.maxMeshWorkGroupSizeX_NV                  = 32;
    result.maxMeshWorkGroupSizeY_NV                  = 1;
    result.maxMeshWorkGroupSizeZ_NV                  = 1;
    result.maxTaskWorkGroupSizeX_NV                  = 32;
    result.maxTaskWorkGroupSizeY_NV                  = 1;
    result.maxTaskWorkGroupSizeZ_NV                  = 1;
    result.maxMeshViewCountNV                        = 4;

    result.limits = {};

    result.limits.nonInductiveForLoops                 = 1;
    result.limits.whileLoops                           = 1;
    result.limits.doWhileLoops                         = 1;
    result.limits.generalUniformIndexing               = 1;
    result.limits.generalAttributeMatrixVectorIndexing = 1;
    result.limits.generalVaryingIndexing               = 1;
    result.limits.generalSamplerIndexing               = 1;
    result.limits.generalVariableIndexing              = 1;
    result.limits.generalConstantMatrixVectorIndexing  = 1;

    return result;
}

} // namespace GVK
