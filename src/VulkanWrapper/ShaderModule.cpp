#include "ShaderModule.hpp"
#include "Assert.hpp"
#include "ShaderReflection.hpp"
#include "Utils.hpp"

#include <iostream>
#include <shaderc/shaderc.hpp>
#include <spirv_cross.hpp>
#include <spirv_reflect.hpp>


static VkShaderStageFlagBits GetShaderStageFromExtension (const std::string& extension)
{
    if (extension == ".vert") {
        return VK_SHADER_STAGE_VERTEX_BIT;
    } else if (extension == ".frag") {
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    } else if (extension == ".geom") {
        return VK_SHADER_STAGE_GEOMETRY_BIT;
    } else if (extension == ".comp") {
        return VK_SHADER_STAGE_COMPUTE_BIT;
    } else if (extension == ".tesc") {
        return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    } else if (extension == ".tese") {
        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    }

    throw std::runtime_error ("unexpected extension");
}


static shaderc_shader_kind GetShaderKindFromExtension (const std::string& extension)
{
    if (extension == ".vert") {
        return shaderc_vertex_shader;
    } else if (extension == ".frag") {
        return shaderc_fragment_shader;
    } else if (extension == ".geom") {
        return shaderc_geometry_shader;
    } else if (extension == ".comp") {
        return shaderc_compute_shader;
    } else if (extension == ".tesc") {
        return shaderc_tess_control_shader;
    } else if (extension == ".tese") {
        return shaderc_tess_evaluation_shader;
    }

    throw std::runtime_error ("unexpected extension");
}


static std::optional<std::vector<uint32_t>> CompileShader (const std::filesystem::path& fileLocation,
                                                           shaderc_optimization_level   optimizationLevel = shaderc_optimization_level_zero)
{
    std::optional<std::string> fileContents = Utils::ReadTextFile (fileLocation);
    if (ERROR (!fileContents.has_value ())) {
        return std::nullopt;
    }

    std::cout << "compiling " << fileLocation.string () << "... ";

    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;

    options.AddMacroDefinition ("MY_DEFINE", "1");
    options.SetOptimizationLevel (optimizationLevel);
    options.SetGenerateDebugInfo ();

    const shaderc_shader_kind shaderKind = GetShaderKindFromExtension (fileLocation.extension ().u8string ());

    // #define DEBUG_COMPILE_ALL

    shaderc::SpvCompilationResult binaryResult = compiler.CompileGlslToSpv (*fileContents, shaderKind, fileLocation.u8string ().c_str (), options);
    std::vector<uint32_t>         binary (binaryResult.cbegin (), binaryResult.cend ());

    if (binaryResult.GetCompilationStatus () != shaderc_compilation_status_success) {
        std::cout << binaryResult.GetErrorMessage () << std::endl;
        return std::nullopt;
    }

#ifdef DEBUG_COMPILE_ALL
    shaderc::AssemblyCompilationResult           asemblyResult      = compiler.CompileGlslToSpvAssembly (*fileContents, shaderKind, fileLocation.u8string ().c_str (), options);
    shaderc::PreprocessedSourceCompilationResult preprocessedResult = compiler.PreprocessGlsl (*fileContents, shaderKind, fileLocation.u8string ().c_str (), options);

    std::string assembly (asemblyResult.cbegin (), asemblyResult.cend ());
    std::string preps (preprocessedResult.cbegin (), preprocessedResult.cend ());
#endif

    std::cout << "done" << std::endl;

    return binary;
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

ShaderModule::ShaderModule (ReadMode mode, VkDevice device, VkShaderModule handle, const std::filesystem::path& fileLocation, const std::vector<uint32_t>& binary)
    : readMode (readMode)
    , device (device)
    , handle (handle)
    , fileLocation (fileLocation)
    , binary (binary)
{
}


ShaderModule::U ShaderModule::CreateFromBinary (VkDevice device, const std::filesystem::path& fileLocation)
{
    std::optional<std::vector<char>> binaryC = Utils::ReadBinaryFile (fileLocation);
    std::optional<std::vector<uint32_t>> binary = Utils::ReadBinaryFile4Byte (fileLocation);
    if (ERROR (!binary.has_value ())) {
        throw std::runtime_error ("failed to read shader");
    }

    VkShaderModule handle = CreateShaderModule (device, *binary);

    return ShaderModule::Create (ReadMode::Binary, device, handle, fileLocation, *binary);
}


ShaderModule::U ShaderModule::CreateFromSource (VkDevice device, const std::filesystem::path& fileLocation)
{
    std::optional<std::vector<uint32_t>> binary = CompileShader (fileLocation);
    if (ERROR (!binary.has_value ())) {
        throw std::runtime_error ("failed to compile shader");
    }

    VkShaderModule handle = CreateShaderModule (device, *binary);

    return ShaderModule::Create (ReadMode::Source, device, handle, fileLocation, *binary);
}

ShaderModule::~ShaderModule ()
{
    vkDestroyShaderModule (device, handle, nullptr);
}


VkPipelineShaderStageCreateInfo ShaderModule::GetShaderStageCreateInfo () const
{
    VkPipelineShaderStageCreateInfo result = {};
    result.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    result.stage                           = GetShaderStageFromExtension (fileLocation.extension ().u8string ());
    result.module                          = handle;
    result.pName                           = "main";
    return result;
}


void ShaderModule::Reload ()
{
    throw std::runtime_error ("unimplemented");
}
