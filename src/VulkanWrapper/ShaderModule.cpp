#include "ShaderModule.hpp"
#include "Assert.hpp"
#include "ShaderReflection.hpp"
#include "Utils.hpp"

#include <array>
#include <iostream>
#include <shaderc/shaderc.hpp>

#include <ResourceLimits.h>
#include <glslang_c_interface.h>


class ShaderKindInfo final {
public:
    const shaderc_shader_kind      shc;
    const std::string              extension;
    const VkShaderStageFlagBits    vkflag;
    const ShaderModule::ShaderKind shaderKind;
    const glslang_stage_t          glslang_stage;

private:
    ShaderKindInfo () = default;

public:
    static const ShaderKindInfo FromExtension (const std::string&);
    static const ShaderKindInfo FromVk (VkShaderStageFlagBits);
    static const ShaderKindInfo FromShaderc (shaderc_shader_kind);
    static const ShaderKindInfo FromShaderKind (ShaderModule::ShaderKind);
    static const ShaderKindInfo FromGlslang (glslang_stage_t);

private:
    static const ShaderKindInfo Find (const std::function<bool (const ShaderKindInfo&)>&);
    static const ShaderKindInfo vert, tesc, tese, geom, frag, comp;

    static const std::array<ShaderKindInfo, 2> allShaderKinds;
};

const ShaderKindInfo ShaderKindInfo::vert {shaderc_vertex_shader, ".vert", VK_SHADER_STAGE_VERTEX_BIT, ShaderModule::ShaderKind::Vertex, GLSLANG_STAGE_VERTEX};
const ShaderKindInfo ShaderKindInfo::tesc {shaderc_tess_control_shader, ".tesc", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, ShaderModule::ShaderKind::TessellationControl, GLSLANG_STAGE_TESSCONTROL};
const ShaderKindInfo ShaderKindInfo::tese {shaderc_tess_evaluation_shader, ".tese", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, ShaderModule::ShaderKind::TessellationEvaluation, GLSLANG_STAGE_TESSEVALUATION};
const ShaderKindInfo ShaderKindInfo::geom {shaderc_geometry_shader, ".geom", VK_SHADER_STAGE_GEOMETRY_BIT, ShaderModule::ShaderKind::Geometry, GLSLANG_STAGE_GEOMETRY};
const ShaderKindInfo ShaderKindInfo::frag {shaderc_fragment_shader, ".frag", VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModule::ShaderKind::Fragment, GLSLANG_STAGE_FRAGMENT};
const ShaderKindInfo ShaderKindInfo::comp {shaderc_compute_shader, ".comp", VK_SHADER_STAGE_COMPUTE_BIT, ShaderModule::ShaderKind::Compute, GLSLANG_STAGE_COMPUTE};

const std::array<ShaderKindInfo, 2> ShaderKindInfo::allShaderKinds ({ShaderKindInfo::vert, ShaderKindInfo::frag});


const ShaderKindInfo ShaderKindInfo::Find (const std::function<bool (const ShaderKindInfo&)>& callback)
{
    for (auto& s : allShaderKinds) {
        if (callback (s)) {
            return s;
        }
    }
    throw std::runtime_error ("couldnt find shader info");
}

const ShaderKindInfo ShaderKindInfo::FromExtension (const std::string& ext)
{
    return Find ([&] (auto& s) {
        return s.extension == ext;
    });
}

const ShaderKindInfo ShaderKindInfo::FromVk (VkShaderStageFlagBits flag)
{
    return Find ([&] (auto& s) {
        return s.vkflag == flag;
    });
}


const ShaderKindInfo ShaderKindInfo::FromShaderc (shaderc_shader_kind sc)
{
    return Find ([&] (auto& s) {
        return s.shc == sc;
    });
}

const ShaderKindInfo ShaderKindInfo::FromShaderKind (ShaderModule::ShaderKind sk)
{
    return Find ([&] (auto& s) {
        return s.shaderKind == sk;
    });
}

const ShaderKindInfo ShaderKindInfo::FromGlslang (glslang_stage_t glslang_stage)
{
    return Find ([&] (auto& s) {
        return s.glslang_stage == glslang_stage;
    });
}


template<typename ClassType, auto CreateFunc, auto DestroyFunc>
struct GlslangClassWrapper final {
    ClassType* handle;

    template<typename... ARGS>
    GlslangClassWrapper (ARGS&&... args)
        : handle (CreateFunc (std::forward<ARGS> (args)...))
    {
    }

    ~GlslangClassWrapper ()
    {
        DestroyFunc (handle);
        handle = nullptr;
    }

    operator ClassType* () const
    {
        return handle;
    }
};

using GlslangShader  = GlslangClassWrapper<glslang_shader_t, glslang_shader_create, glslang_shader_delete>;
using GlslangProgram = GlslangClassWrapper<glslang_program_t, glslang_program_create, glslang_program_delete>;


std::optional<std::vector<uint32_t>> CompileWithGlslang (const std::string& sourceCode, const ShaderKindInfo& shaderKind)
{
    const TBuiltInResource limits = {
        /*.maxLights = */ 8,        // From OpenGL 3.0 table 6.46.
        /*.maxClipPlanes = */ 6,    // From OpenGL 3.0 table 6.46.
        /*.maxTextureUnits = */ 2,  // From OpenGL 3.0 table 6.50.
        /*.maxTextureCoords = */ 8, // From OpenGL 3.0 table 6.50.
        /*.maxVertexAttribs = */ 16,
        /*.maxVertexUniformComponents = */ 4096,
        /*.maxVaryingFloats = */ 60, // From OpenGLES 3.1 table 6.44.
        /*.maxVertexTextureImageUnits = */ 16,
        /*.maxCombinedTextureImageUnits = */ 80,
        /*.maxTextureImageUnits = */ 16,
        /*.maxFragmentUniformComponents = */ 1024,

        // glslang has 32 maxDrawBuffers.
        // Pixel phone Vulkan driver in Android N has 8
        // maxFragmentOutputAttachments.
        /*.maxDrawBuffers = */ 8,

        /*.maxVertexUniformVectors = */ 256,
        /*.maxVaryingVectors = */ 15, // From OpenGLES 3.1 table 6.44.
        /*.maxFragmentUniformVectors = */ 256,
        /*.maxVertexOutputVectors = */ 16,  // maxVertexOutputComponents / 4
        /*.maxFragmentInputVectors = */ 15, // maxFragmentInputComponents / 4
        /*.minProgramTexelOffset = */ -8,
        /*.maxProgramTexelOffset = */ 7,
        /*.maxClipDistances = */ 8,
        /*.maxComputeWorkGroupCountX = */ 65535,
        /*.maxComputeWorkGroupCountY = */ 65535,
        /*.maxComputeWorkGroupCountZ = */ 65535,
        /*.maxComputeWorkGroupSizeX = */ 1024,
        /*.maxComputeWorkGroupSizeX = */ 1024,
        /*.maxComputeWorkGroupSizeZ = */ 64,
        /*.maxComputeUniformComponents = */ 512,
        /*.maxComputeTextureImageUnits = */ 16,
        /*.maxComputeImageUniforms = */ 8,
        /*.maxComputeAtomicCounters = */ 8,
        /*.maxComputeAtomicCounterBuffers = */ 1, // From OpenGLES 3.1 Table 6.43
        /*.maxVaryingComponents = */ 60,
        /*.maxVertexOutputComponents = */ 64,
        /*.maxGeometryInputComponents = */ 64,
        /*.maxGeometryOutputComponents = */ 128,
        /*.maxFragmentInputComponents = */ 128,
        /*.maxImageUnits = */ 8, // This does not seem to be defined anywhere,
                                 // set to ImageUnits.
        /*.maxCombinedImageUnitsAndFragmentOutputs = */ 8,
        /*.maxCombinedShaderOutputResources = */ 8,
        /*.maxImageSamples = */ 0,
        /*.maxVertexImageUniforms = */ 0,
        /*.maxTessControlImageUniforms = */ 0,
        /*.maxTessEvaluationImageUniforms = */ 0,
        /*.maxGeometryImageUniforms = */ 0,
        /*.maxFragmentImageUniforms = */ 8,
        /*.maxCombinedImageUniforms = */ 8,
        /*.maxGeometryTextureImageUnits = */ 16,
        /*.maxGeometryOutputVertices = */ 256,
        /*.maxGeometryTotalOutputComponents = */ 1024,
        /*.maxGeometryUniformComponents = */ 512,
        /*.maxGeometryVaryingComponents = */ 60, // Does not seem to be defined
                                                 // anywhere, set equal to
                                                 // maxVaryingComponents.
        /*.maxTessControlInputComponents = */ 128,
        /*.maxTessControlOutputComponents = */ 128,
        /*.maxTessControlTextureImageUnits = */ 16,
        /*.maxTessControlUniformComponents = */ 1024,
        /*.maxTessControlTotalOutputComponents = */ 4096,
        /*.maxTessEvaluationInputComponents = */ 128,
        /*.maxTessEvaluationOutputComponents = */ 128,
        /*.maxTessEvaluationTextureImageUnits = */ 16,
        /*.maxTessEvaluationUniformComponents = */ 1024,
        /*.maxTessPatchComponents = */ 120,
        /*.maxPatchVertices = */ 32,
        /*.maxTessGenLevel = */ 64,
        /*.maxViewports = */ 16,
        /*.maxVertexAtomicCounters = */ 0,
        /*.maxTessControlAtomicCounters = */ 0,
        /*.maxTessEvaluationAtomicCounters = */ 0,
        /*.maxGeometryAtomicCounters = */ 0,
        /*.maxFragmentAtomicCounters = */ 8,
        /*.maxCombinedAtomicCounters = */ 8,
        /*.maxAtomicCounterBindings = */ 1,
        /*.maxVertexAtomicCounterBuffers = */ 0, // From OpenGLES 3.1 Table 6.41.

        // ARB_shader_atomic_counters.
        /*.maxTessControlAtomicCounterBuffers = */ 0,
        /*.maxTessEvaluationAtomicCounterBuffers = */ 0,
        /*.maxGeometryAtomicCounterBuffers = */ 0,
        // /ARB_shader_atomic_counters.

        /*.maxFragmentAtomicCounterBuffers = */ 0, // From OpenGLES 3.1 Table 6.43.
        /*.maxCombinedAtomicCounterBuffers = */ 1,
        /*.maxAtomicCounterBufferSize = */ 32,
        /*.maxTransformFeedbackBuffers = */ 4,
        /*.maxTransformFeedbackInterleavedComponents = */ 64,
        /*.maxCullDistances = */ 8,                // ARB_cull_distance.
        /*.maxCombinedClipAndCullDistances = */ 8, // ARB_cull_distance.
        /*.maxSamples = */ 4,
        /* .maxMeshOutputVerticesNV = */ 256,
        /* .maxMeshOutputPrimitivesNV = */ 512,
        /* .maxMeshWorkGroupSizeX_NV = */ 32,
        /* .maxMeshWorkGroupSizeY_NV = */ 1,
        /* .maxMeshWorkGroupSizeZ_NV = */ 1,
        /* .maxTaskWorkGroupSizeX_NV = */ 32,
        /* .maxTaskWorkGroupSizeY_NV = */ 1,
        /* .maxTaskWorkGroupSizeZ_NV = */ 1,
        /* .maxMeshViewCountNV = */ 4,

        // This is the glslang TLimits structure.
        // It defines whether or not the following features are enabled.
        // We want them to all be enabled.
        /*.limits = */ {
            /*.nonInductiveForLoops = */ 1,
            /*.whileLoops = */ 1,
            /*.doWhileLoops = */ 1,
            /*.generalUniformIndexing = */ 1,
            /*.generalAttributeMatrixVectorIndexing = */ 1,
            /*.generalVaryingIndexing = */ 1,
            /*.generalSamplerIndexing = */ 1,
            /*.generalVariableIndexing = */ 1,
            /*.generalConstantMatrixVectorIndexing = */ 1,
        }};
    glslang_input_t input                   = {};
    input.language                          = GLSLANG_SOURCE_GLSL;
    input.stage                             = shaderKind.glslang_stage;
    input.client                            = GLSLANG_CLIENT_VULKAN;
    input.client_version                    = GLSLANG_TARGET_VULKAN_1_1;
    input.target_language                   = GLSLANG_TARGET_SPV;
    input.target_language_version           = GLSLANG_TARGET_SPV_1_3;
    input.code                              = sourceCode.data ();
    input.default_version                   = 110;
    input.default_profile                   = GLSLANG_NO_PROFILE;
    input.force_default_version_and_profile = false;
    input.forward_compatible                = false;
    input.messages                          = GLSLANG_MSG_DEFAULT_BIT;
    input.resource                          = &limits;

    glslang_initialize_process ();

    GlslangShader shader (&input);

    if (ERROR (!glslang_shader_preprocess (shader, &input))) {
        return std::nullopt;
        // use glslang_shader_get_info_log() and glslang_shader_get_info_debug_log()
    }

    if (ERROR (!glslang_shader_parse (shader, &input))) {
        return std::nullopt;
        // use glslang_shader_get_info_log() and glslang_shader_get_info_debug_log()
    }

    GlslangProgram program;
    glslang_program_add_shader (program, shader);

    if (ERROR (!glslang_program_link (program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))) {
        return std::nullopt;
        // use glslang_program_get_info_log() and glslang_program_get_info_debug_log();
    }

    glslang_program_SPIRV_generate (program, input.stage);

    if (glslang_program_SPIRV_get_messages (program)) {
        printf ("%s", glslang_program_SPIRV_get_messages (program));
    }

    const uint64_t codeSize = glslang_program_SPIRV_get_size (program);

    std::vector<uint32_t> result (codeSize + (sizeof (uint32_t) - codeSize % sizeof (uint32_t)), 0);

    std::memcpy (result.data (), glslang_program_SPIRV_get_ptr (program), glslang_program_SPIRV_get_size (program));

    return result;
}


static std::optional<std::vector<uint32_t>> CompileShaderFromString (const std::string&         shaderSource,
                                                                     shaderc_shader_kind        shaderKind,
                                                                     shaderc_optimization_level optimizationLevel = shaderc_optimization_level_zero)
{
    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;

    options.AddMacroDefinition ("MY_DEFINE", "1");
    options.SetOptimizationLevel (optimizationLevel);
    options.SetGenerateDebugInfo ();

    // #define DEBUG_COMPILE_ALL

    shaderc::SpvCompilationResult binaryResult = compiler.CompileGlslToSpv (shaderSource, shaderKind, shaderSource.c_str (), options);
    std::vector<uint32_t>         binary (binaryResult.cbegin (), binaryResult.cend ());

    if (binaryResult.GetCompilationStatus () != shaderc_compilation_status_success) {
        std::cerr << binaryResult.GetErrorMessage () << std::endl;
        return std::nullopt;
    }

#ifdef DEBUG_COMPILE_ALL
    shaderc::AssemblyCompilationResult           asemblyResult      = compiler.CompileGlslToSpvAssembly (*fileContents, shaderKind, fileLocation.u8string ().c_str (), options);
    shaderc::PreprocessedSourceCompilationResult preprocessedResult = compiler.PreprocessGlsl (*fileContents, shaderKind, fileLocation.u8string ().c_str (), options);

    std::string assembly (asemblyResult.cbegin (), asemblyResult.cend ());
    std::string preps (preprocessedResult.cbegin (), preprocessedResult.cend ());
#endif

    return binary;
}


static std::optional<std::vector<uint32_t>> CompileShaderFromFile (const std::filesystem::path& fileLocation,
                                                                   shaderc_optimization_level   optimizationLevel = shaderc_optimization_level_zero)
{
    std::optional<std::string> fileContents = Utils::ReadTextFile (fileLocation);
    if (ERROR (!fileContents.has_value ())) {
        return std::nullopt;
    }

    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;

    options.AddMacroDefinition ("MY_DEFINE", "1");
    options.SetOptimizationLevel (optimizationLevel);
    options.SetGenerateDebugInfo ();

    //:mreturn CompileWithGlslang (*fileContents, ShaderKindInfo::FromExtension (fileLocation.extension ().u8string ()));

    const shaderc_shader_kind shaderKind = ShaderKindInfo::FromExtension (fileLocation.extension ().u8string ()).shc;
    return CompileShaderFromString (*fileContents, shaderKind, optimizationLevel);
}


static VkShaderModule CreateShaderModule (VkDevice device, const std::vector<uint32_t>& binary)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize                 = static_cast<uint32_t> (binary.size () * sizeof (uint32_t)); // size must be in bytes
    createInfo.pCode                    = reinterpret_cast<const uint32_t*> (binary.data ());

    VkShaderModule result = VK_NULL_HANDLE;
    if (ERROR (vkCreateShaderModule (device, &createInfo, nullptr, &result) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create shader module");
    }

    return result;
}

ShaderModule::ShaderModule (ShaderModule::ShaderKind shaderKind, ReadMode mode, VkDevice device, VkShaderModule handle, const std::filesystem::path& fileLocation, const std::vector<uint32_t>& binary)
    : readMode (readMode)
    , shaderKind (shaderKind)
    , device (device)
    , handle (handle)
    , fileLocation (fileLocation)
    , binary (binary)
{
}


ShaderModule::U ShaderModule::CreateFromBinary (VkDevice device, const std::filesystem::path& fileLocation)
{
    std::optional<std::vector<char>>     binaryC = Utils::ReadBinaryFile (fileLocation);
    std::optional<std::vector<uint32_t>> binary  = Utils::ReadBinaryFile4Byte (fileLocation);
    if (ERROR (!binary.has_value ())) {
        throw std::runtime_error ("failed to read shader");
    }

    VkShaderModule handle = CreateShaderModule (device, *binary);

    return ShaderModule::Create (ShaderKindInfo::FromExtension (fileLocation.extension ().u8string ()).shaderKind, ReadMode::Binary, device, handle, fileLocation, *binary);
}


ShaderModule::U ShaderModule::CreateFromSource (VkDevice device, const std::filesystem::path& fileLocation)
{
    std::optional<std::vector<uint32_t>> binary = CompileShaderFromFile (fileLocation);
    if (ERROR (!binary.has_value ())) {
        throw std::runtime_error ("failed to compile shader");
    }

    VkShaderModule handle = CreateShaderModule (device, *binary);

    return ShaderModule::Create (ShaderKindInfo::FromExtension (fileLocation.extension ().u8string ()).shaderKind, ReadMode::Source, device, handle, fileLocation, *binary);
}


ShaderModule::U ShaderModule::CreateFromString (VkDevice device, const std::string& shaderSource, ShaderKind shaderKind)
{
    //std::vector<uint32_t> binary = CompileWithGlslang (shaderSource, ShaderKindInfo::FromShaderKind (shaderKind));

    std::optional<std::vector<uint32_t>> binary = CompileShaderFromString (shaderSource, ShaderKindInfo::FromShaderKind (shaderKind).shc);
    if (ERROR (!binary.has_value ())) {
        throw std::runtime_error ("failed to compile shader");
    }

    VkShaderModule handle = CreateShaderModule (device, *binary);

    return ShaderModule::Create (shaderKind, ReadMode::String, device, handle, "", *binary);
}

ShaderModule::~ShaderModule ()
{
    vkDestroyShaderModule (device, handle, nullptr);
    handle = VK_NULL_HANDLE;
}


VkPipelineShaderStageCreateInfo ShaderModule::GetShaderStageCreateInfo () const
{
    VkPipelineShaderStageCreateInfo result = {};
    result.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    result.stage                           = ShaderKindInfo::FromShaderKind (shaderKind).vkflag;
    result.module                          = handle;
    result.pName                           = "main";
    return result;
}


void ShaderModule::Reload ()
{
    ASSERT (readMode != ReadMode::String);
    throw std::runtime_error ("unimplemented");
}
