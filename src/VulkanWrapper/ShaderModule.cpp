#include "ShaderModule.hpp"
#include "Assert.hpp"
#include "ShaderReflection.hpp"
#include "Utils.hpp"

#include <array>
#include <iostream>
#include <shaderc/shaderc.hpp>
#include <spirv_cross.hpp>
#include <spirv_reflect.hpp>


class ShaderKindInfo final {
public:
    const shaderc_shader_kind      shc;
    const std::string              extension;
    const VkShaderStageFlagBits    vkflag;
    const ShaderModule::ShaderKind shaderKind;

private:
    ShaderKindInfo () = default;

public:
    static const ShaderKindInfo FromExtension (const std::string&);
    static const ShaderKindInfo FromVk (VkShaderStageFlagBits);
    static const ShaderKindInfo FromShaderc (shaderc_shader_kind);
    static const ShaderKindInfo FromShaderKind (ShaderModule::ShaderKind);

private:
    static const ShaderKindInfo Find (const std::function<bool (const ShaderKindInfo&)>&);
    static const ShaderKindInfo vert, tesc, tese, geom, frag, comp;

    static const std::array<ShaderKindInfo, 2> allShaderKinds;
};

const ShaderKindInfo ShaderKindInfo::vert {shaderc_vertex_shader, ".vert", VK_SHADER_STAGE_VERTEX_BIT, ShaderModule::ShaderKind::Vertex};
const ShaderKindInfo ShaderKindInfo::tesc {shaderc_tess_control_shader, ".tesc", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, ShaderModule::ShaderKind::TessellationControl};
const ShaderKindInfo ShaderKindInfo::tese {shaderc_tess_evaluation_shader, ".tese", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, ShaderModule::ShaderKind::TessellationEvaluation};
const ShaderKindInfo ShaderKindInfo::geom {shaderc_geometry_shader, ".geom", VK_SHADER_STAGE_GEOMETRY_BIT, ShaderModule::ShaderKind::Geometry};
const ShaderKindInfo ShaderKindInfo::frag {shaderc_fragment_shader, ".frag", VK_SHADER_STAGE_FRAGMENT_BIT, ShaderModule::ShaderKind::Fragment};
const ShaderKindInfo ShaderKindInfo::comp {shaderc_compute_shader, ".comp", VK_SHADER_STAGE_COMPUTE_BIT, ShaderModule::ShaderKind::Compute};

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
