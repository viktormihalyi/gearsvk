#include "ShaderModule.hpp"

// from Utils
#include "Utils/Assert.hpp"
#include "Utils/BuildType.hpp"
#include "Utils/CommandLineFlag.hpp"
#include "Utils/FileSystemUtils.hpp"

// from VulkanWrapper
#include "ResourceLimits.hpp"
#include "ShaderReflection.hpp"

// from std
#include <array>

// from glslang
#include "glslang/SPIRV/GlslangToSpv.h"

// from spdlog
#include "spdlog/spdlog.h"

#if 0
static Utils::CommandLineOnOffFlag disableShaderCacheFlag ("--disableShaderCache");
#endif

namespace GVK {


class ShaderKindDescriptor final {
public:
    const char*           extension;
    VkShaderStageFlagBits vkflag;
    ShaderKind            shaderKind;
    EShLanguage           esh;
    const char*           displayName;

private:
    ShaderKindDescriptor (const char*           extension,
                       VkShaderStageFlagBits vkflag,
                       ShaderKind            shaderKind,
                       EShLanguage           esh,
                       const char*           displayName);

public:
    static std::optional<ShaderKindDescriptor> FromExtension (const std::string&);
    static std::optional<ShaderKindDescriptor> FromVk (VkShaderStageFlagBits);
    static std::optional<ShaderKindDescriptor> FromShaderKind (ShaderKind);
    static std::optional<ShaderKindDescriptor> FromEsh (EShLanguage);

private:
    static std::optional<ShaderKindDescriptor> Find (const std::function<bool (const ShaderKindDescriptor&)>&);
    static const ShaderKindDescriptor vert, tesc, tese, geom, frag, comp;

    static const std::array<ShaderKindDescriptor, 6> allShaderKinds;
};


ShaderKindDescriptor::ShaderKindDescriptor (const char*           extension,
                                      VkShaderStageFlagBits vkflag,
                                      ShaderKind            shaderKind,
                                      EShLanguage           esh,
                                      const char*           displayName)
    : extension { extension }
    , vkflag { vkflag }
    , shaderKind { shaderKind }
    , esh { esh }
    , displayName { displayName }
{
}


const ShaderKindDescriptor ShaderKindDescriptor::vert {
    ".vert",
    VK_SHADER_STAGE_VERTEX_BIT,
    ShaderKind::Vertex,
    EShLangVertex,
    "Vertex Shader"
};


const ShaderKindDescriptor ShaderKindDescriptor::tesc {
    ".tesc",
    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    ShaderKind::TessellationControl,
    EShLangTessControl,
    "Tessellation Control Shader"
};


const ShaderKindDescriptor ShaderKindDescriptor::tese {
    ".tese",
    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    ShaderKind::TessellationEvaluation,
    EShLangTessEvaluation,
    "Tessellation Evaluation Shader"
};


const ShaderKindDescriptor ShaderKindDescriptor::geom {
    ".geom",
    VK_SHADER_STAGE_GEOMETRY_BIT,
    ShaderKind::Geometry,
    EShLangGeometry,
    "Geometry Shader"
};


const ShaderKindDescriptor ShaderKindDescriptor::frag {
    ".frag",
    VK_SHADER_STAGE_FRAGMENT_BIT,
    ShaderKind::Fragment,
    EShLangFragment,
    "Fragment Shader"
};


const ShaderKindDescriptor ShaderKindDescriptor::comp {
    ".comp",
    VK_SHADER_STAGE_COMPUTE_BIT,
    ShaderKind::Compute,
    EShLangCompute,
    "Compute Shader"
};


const std::array<ShaderKindDescriptor, 6> ShaderKindDescriptor::allShaderKinds ({
    ShaderKindDescriptor::vert,
    ShaderKindDescriptor::tese,
    ShaderKindDescriptor::tesc,
    ShaderKindDescriptor::geom,
    ShaderKindDescriptor::frag,
    ShaderKindDescriptor::comp,
});


std::optional<ShaderKindDescriptor> ShaderKindDescriptor::Find (const std::function<bool (const ShaderKindDescriptor&)>& callback)
{
    for (auto& s : allShaderKinds) {
        if (callback (s)) {
            return s;
        }
    }

    return std::nullopt;
}

std::optional<ShaderKindDescriptor> ShaderKindDescriptor::FromExtension (const std::string& ext)
{
    return Find ([&] (auto& s) {
        return s.extension == ext;
    });
}

std::optional<ShaderKindDescriptor> ShaderKindDescriptor::FromVk (VkShaderStageFlagBits flag)
{
    return Find ([&] (auto& s) {
        return s.vkflag == flag;
    });
}

std::optional<ShaderKindDescriptor> ShaderKindDescriptor::FromShaderKind (ShaderKind sk)
{
    return Find ([&] (auto& s) {
        return s.shaderKind == sk;
    });
}

std::optional<ShaderKindDescriptor> ShaderKindDescriptor::FromEsh (EShLanguage e)
{
    return Find ([&] (auto& s) {
        return s.esh == e;
    });
}


#if 0

class ShaderCache {
private:
    const std::filesystem::path tempFolder;

public:
    ShaderCache (const std::string& typeName)
        : tempFolder (std::filesystem::temp_directory_path () / "GearsVk" / "ShaderCache" / typeName)
    {
    }

private:
    std::string GetShaderSourceHash (const std::string& sourceCode) const
    {
        std::hash<std::string> sourceCodeHasher;

        const size_t sourceCodeHash = sourceCodeHasher (sourceCode);

        return std::to_string (sourceCodeHash);
    };

public:
    std::optional<std::vector<uint32_t>> Load (const std::string& sourceCode) const
    {
        if (disableShaderCacheFlag.IsFlagOn ()) {
            return std::nullopt;
        }

        const std::string hashString = GetShaderSourceHash (sourceCode);

        const std::string cachedCodeFileName   = hashString + "_code.txt";
        const std::string cachedBinaryFileName = hashString + "_binary.txt";

        if (!std::filesystem::exists (tempFolder / cachedCodeFileName)) {
            return std::nullopt;
        }

        const std::optional<std::string> actualCode = Utils::ReadTextFile (tempFolder / cachedCodeFileName);
        if (GVK_ERROR (!actualCode)) {
            return std::nullopt;
        }

        if (sourceCode != *actualCode) {
            return std::nullopt;
        }

        if (!std::filesystem::exists (tempFolder / cachedBinaryFileName)) {
            return std::nullopt;
        }

        std::optional<std::vector<char>> binaryC = Utils::ReadBinaryFile (tempFolder / cachedBinaryFileName);
        if (GVK_ERROR (!binaryC.has_value ())) {
            return std::nullopt;
        }

        std::vector<uint32_t> code;
        code.resize (binaryC->size () / sizeof (uint32_t));
        memcpy (code.data (), binaryC->data (), binaryC->size ());

        return code;
    }

    bool Save (const std::string& sourceCode, const std::vector<uint32_t>& binary) const
    {
        if (disableShaderCacheFlag.IsFlagOn ()) {
            return false;
        }

        const std::string hashString = GetShaderSourceHash (sourceCode);

        const std::string cachedCodeFileName   = hashString + "_code.txt";
        const std::string cachedBinaryFileName = hashString + "_binary.txt";

        if (std::filesystem::exists (tempFolder / cachedCodeFileName) || std::filesystem::exists (tempFolder / cachedBinaryFileName)) {
            return false;
        }

        GVK_VERIFY (Utils::WriteTextFile (tempFolder / cachedCodeFileName, sourceCode));
        GVK_VERIFY (Utils::WriteBinaryFile (tempFolder / cachedBinaryFileName, binary.data (), binary.size () * sizeof (uint32_t)));

        return true;
    }
};

ShaderCache debugShaderCache ("Debug");
ShaderCache releaseShaderCache ("Release");

#endif


static std::vector<uint32_t> CompileWithGlslangCppInterface (const std::string& sourceCode, const ShaderKindDescriptor& shaderKind)
{
    static bool init = false;
    if (!init) {
        init = true;
        glslang::InitializeProcess ();
    }

    const char* const                       sourceCstr                  = sourceCode.c_str ();
    const int                               ClientInputSemanticsVersion = 100;
    const glslang::EShTargetClientVersion   VulkanClientVersion         = glslang::EShTargetVulkan_1_2;
    const glslang::EShTargetLanguageVersion TargetVersion               = glslang::EShTargetSpv_1_5;
    const EShMessages                       messages                    = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo); // TODO remove debug info from release builds
    const TBuiltInResource                  resources                   = GetDefaultResourceLimits (); // TODO use DefaultTBuiltInResource ?

    glslang::TShader shader (shaderKind.esh);
    shader.setStrings (&sourceCstr, 1);
    shader.setEnvInput (glslang::EShSourceGlsl, shaderKind.esh, glslang::EShClientVulkan, ClientInputSemanticsVersion);
    shader.setEnvClient (glslang::EShClientVulkan, VulkanClientVersion);
    shader.setEnvTarget (glslang::EShTargetSpv, TargetVersion);

    if (!shader.parse (&resources, 100, false, messages)) {
        throw ShaderCompileException (std::string { "Failed to parse " } + shaderKind.displayName + ":\n"
                                     + "================================== GLSL CODE BEGIN ==================================\n"
                                     + sourceCode + "\n"
                                     + "================================== GLSL CODE END ====================================\n"
                                     + shader.getInfoLog ());
    }

    glslang::TProgram program;
    program.addShader (&shader);

    if (!program.link (EShMsgDefault)) {
        throw ShaderCompileException (std::string { "Failed to link " } + shaderKind.displayName + ":\n"
                                     + "================================== GLSL CODE BEGIN ==================================\n"
                                     + sourceCode + "\n"
                                     + "================================== GLSL CODE END ====================================\n"
                                     + shader.getInfoLog ());
    }

    spv::SpvBuildLogger logger;
    glslang::SpvOptions spvOptions;
    spvOptions.generateDebugInfo = false;
    spvOptions.stripDebugInfo    = false;
    spvOptions.disableOptimizer  = true;
    spvOptions.optimizeSize      = false;
    spvOptions.disassemble       = false;
    spvOptions.validate          = false;

    std::vector<uint32_t> spirvBinary;
    glslang::GlslangToSpv (*program.getIntermediate (shaderKind.esh), spirvBinary, &logger, &spvOptions);

    const std::string loggerMessages = logger.getAllMessages ();
    if (GVK_ERROR (!loggerMessages.empty ()))
        spdlog::error ("{}", loggerMessages);

    return spirvBinary;
}


static std::vector<uint32_t> CompileFromSourceCode (const std::string& shaderSource, const ShaderKindDescriptor& shaderKind)
{
#if 0
    ShaderCache& shaderCache = IsDebugBuild ? debugShaderCache : releaseShaderCache;

    std::optional<std::vector<uint32_t>> cachedBinary = shaderCache.Load (shaderSource);
    if (cachedBinary.has_value ()) {
        return *cachedBinary;
    }
#endif

    try {
        const std::vector<uint32_t> result = CompileWithGlslangCppInterface (shaderSource, shaderKind);
#if 0
        shaderCache.Save (shaderSource, result);
#endif
        return result;

    } catch (ShaderCompileException& ex) {
        throw;
    }
}


static VkShaderModule CreateShaderModuleImpl (VkDevice device, const std::vector<uint32_t>& binary)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize                 = static_cast<uint32_t> (binary.size () * sizeof (uint32_t)); // size must be in bytes
    createInfo.pCode                    = reinterpret_cast<const uint32_t*> (binary.data ());

    VkShaderModule result = VK_NULL_HANDLE;
    if (GVK_ERROR (vkCreateShaderModule (device, &createInfo, nullptr, &result) != VK_SUCCESS)) {
        spdlog::critical ("VkShaderModule creation failed.");
        throw std::runtime_error ("failed to create shader module");
    }
    
    return result;
}


static VkShaderModule CreateShaderModuleImpl (VkDevice device, const std::vector<char>& binary)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize                 = binary.size ();
    createInfo.pCode                    = reinterpret_cast<const uint32_t*> (binary.data ());

    VkShaderModule result = VK_NULL_HANDLE;
    if (GVK_ERROR (vkCreateShaderModule (device, &createInfo, nullptr, &result) != VK_SUCCESS)) {
        spdlog::critical ("VkShaderModule creation failed.");
        throw std::runtime_error ("failed to create shader module");
    }

    return result;
}


ShaderModule::ShaderModule (ShaderKind                   shaderKind,
                            ReadMode                     readMode,
                            VkDevice                     device,
                            VkShaderModule               handle,
                            const std::filesystem::path& fileLocation,
                            const std::vector<uint32_t>& binary,
                            const std::string&           sourceCode)
    : readMode (readMode)
    , shaderKind (shaderKind)
    , device (device)
    , handle (handle)
    , fileLocation (fileLocation)
    , binary (binary)
    , reflection (binary)
    , sourceCode (sourceCode)
{
    spdlog::trace ("VkShaderModule created: {}, uuid: {}.", this->handle, GetUUID ().GetValue ());
}


std::unique_ptr<ShaderModule> ShaderModule::CreateFromSPVFile (VkDevice device, ShaderKind shaderKind, const std::filesystem::path& fileLocation)
{
    std::optional<std::vector<char>> binaryC = Utils::ReadBinaryFile (fileLocation);
    if (GVK_ERROR (!binaryC.has_value ())) {
        throw std::runtime_error ("failed to read shader");
    }

    std::vector<uint32_t> code;
    code.resize (binaryC->size () / sizeof (uint32_t));
    memcpy (code.data (), binaryC->data (), binaryC->size ());

    VkShaderModule handle = CreateShaderModuleImpl (device, *binaryC);

    return std::unique_ptr<ShaderModule> (new ShaderModule (shaderKind, ReadMode::SPVFilePath, device, handle, fileLocation, code, ""));
}


std::unique_ptr<ShaderModule> ShaderModule::CreateFromGLSLFile (VkDevice device, const std::filesystem::path& fileLocation)
{
    std::optional<std::string> fileContents = Utils::ReadTextFile (fileLocation);
    if (GVK_ERROR (!fileContents.has_value ())) {
        throw std::runtime_error ("Failed to read file.");
    }

    std::optional<ShaderKindDescriptor> shaderKindDescriptor = ShaderKindDescriptor::FromExtension (fileLocation.extension ().string ());
    if (GVK_ERROR (!shaderKindDescriptor.has_value ())) {
        throw std::runtime_error ("Unknown shader file extension.");
    }

    std::optional<std::vector<uint32_t>> binary = CompileFromSourceCode (*fileContents, *shaderKindDescriptor);
    if (GVK_ERROR (!binary.has_value ())) {
        throw std::runtime_error ("failed to compile shader");
    }

    VkShaderModule handle = CreateShaderModuleImpl (device, *binary);
    
    return std::unique_ptr<ShaderModule> (new ShaderModule (
        shaderKindDescriptor->shaderKind,
        ReadMode::GLSLFilePath,
        device,
        handle,
        fileLocation,
        *binary,
        *fileContents));
}


std::unique_ptr<ShaderModule> ShaderModule::CreateFromGLSLString (VkDevice device, ShaderKind shaderKind, const std::string& shaderSource)
{
    std::optional<ShaderKindDescriptor> shaderKindDescriptor = ShaderKindDescriptor::FromShaderKind (shaderKind);
    if (GVK_ERROR (!shaderKindDescriptor.has_value ())) {
        throw std::runtime_error ("Unknown shaderkind.");
    }

    std::vector<uint32_t> binary = CompileFromSourceCode (shaderSource, *shaderKindDescriptor);

    VkShaderModule handle = CreateShaderModuleImpl (device, binary);
    
    return std::unique_ptr<ShaderModule> (new ShaderModule (
        shaderKind,
        ReadMode::GLSLString,
        device,
        handle,
        "",
        binary,
        shaderSource));
}


ShaderModule::~ShaderModule ()
{
    vkDestroyShaderModule (device, handle, nullptr);
    handle = nullptr;
}


VkPipelineShaderStageCreateInfo ShaderModule::GetShaderStageCreateInfo () const
{
    VkPipelineShaderStageCreateInfo result = {};
    result.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    result.pNext                           = nullptr;
    result.flags                           = 0;
    result.stage                           = ShaderKindDescriptor::FromShaderKind (shaderKind)->vkflag;
    result.module                          = handle;
    result.pName                           = "main";
    result.pSpecializationInfo             = nullptr;
    return result;
}


ShaderModule::Reflection::Reflection (const std::vector<uint32_t>& binary)
{
    SR::SpirvParser c (binary);

    ubos           = SR::GetUBOsFromBinary (c);
    samplers       = SR::GetSamplersFromBinary (c);
    storageBuffers = SR::GetStorageBuffersFromBinary (c);
    inputs         = SR::GetInputsFromBinary (c);
    outputs        = SR::GetOutputsFromBinary (c);
    subpassInputs  = SR::GetSubpassInputsFromBinary (c);
}


void ShaderModule::Reload ()
{
    if (readMode == ReadMode::GLSLFilePath) {
        vkDestroyShaderModule (device, handle, nullptr);

        std::optional<std::string> fileContents = Utils::ReadTextFile (fileLocation);
        if (GVK_ERROR (!fileContents.has_value ())) {
            throw std::runtime_error ("failed to read shader");
        }

        std::optional<ShaderKindDescriptor> shaderKindDescriptor = ShaderKindDescriptor::FromExtension (fileLocation.extension ().string ());
        if (GVK_ERROR (!shaderKindDescriptor.has_value ())) {
            return;
        }

        std::optional<std::vector<uint32_t>> newBinary = CompileFromSourceCode (*fileContents, *shaderKindDescriptor);
        if (GVK_ERROR (!newBinary.has_value ())) {
            throw std::runtime_error ("failed to compile shader");
        }

        handle = CreateShaderModuleImpl (device, *newBinary);

        binary = *newBinary;

        reflection = Reflection (binary);

        sourceCode = *fileContents;

    } else if (readMode == ReadMode::SPVFilePath) {
        vkDestroyShaderModule (device, handle, nullptr);

        std::optional<std::vector<char>> binaryC = Utils::ReadBinaryFile (fileLocation);
        if (GVK_ERROR (!binaryC.has_value ())) {
            throw std::runtime_error ("failed to read shader");
        }

        std::vector<uint32_t> code;
        code.resize (binaryC->size () / sizeof (uint32_t));
        memcpy (code.data (), binaryC->data (), binaryC->size ());

        handle = CreateShaderModuleImpl (device, *binaryC);

        reflection = Reflection (binary);

        binary = code;

    } else if (readMode == ReadMode::GLSLString) {
        GVK_BREAK_STR ("cannot reload shaders from hard coded strings");

    } else {
        GVK_BREAK_STR ("unknown read mode");
    }
}


std::string ShaderKindToString (ShaderKind shaderKind)
{
    switch (shaderKind) {
        case ShaderKind::Fragment: return "Fragment";
        case ShaderKind::Vertex: return "Vertex";
        case ShaderKind::TessellationControl: return "TessellationControl";
        case ShaderKind::TessellationEvaluation: return "TessellationEvaluation";
        case ShaderKind::Geometry: return "Geometry";
        case ShaderKind::Compute: return "Compute";
        default:
            GVK_BREAK_STR ("unexpected shaderkind type");
            return "";
    }
}

} // namespace GVK