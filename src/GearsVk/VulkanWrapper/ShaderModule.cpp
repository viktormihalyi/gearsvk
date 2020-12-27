#include "ShaderModule.hpp"

// from Utils
#include "Assert.hpp"
#include "Utils.hpp"

// from VulkanWrapper
#include "ResourceLimits.hpp"
#include "ShaderReflection.hpp"

// from std
#include <array>
#include <iostream>

// from glslang
#include <SPIRV/GlslangToSpv.h>
#include <glslang/MachineIndependent/reflection.h>
#include <glslang/Public/ShaderLang.h>


EmptyPreprocessor emptyPreprocessor;


class ShaderKindInfo final {
public:
    const std::string           extension;
    const VkShaderStageFlagBits vkflag;
    const ShaderKind            shaderKind;
    const EShLanguage           esh;

private:
    ShaderKindInfo () = default;

public:
    static const ShaderKindInfo FromExtension (const std::string&);
    static const ShaderKindInfo FromVk (VkShaderStageFlagBits);
    static const ShaderKindInfo FromShaderKind (ShaderKind);
    static const ShaderKindInfo FromEsh (EShLanguage);

private:
    static const ShaderKindInfo Find (const std::function<bool (const ShaderKindInfo&)>&);
    static const ShaderKindInfo vert, tesc, tese, geom, frag, comp;

    static const std::array<ShaderKindInfo, 6> allShaderKinds;
};

const ShaderKindInfo ShaderKindInfo::vert {
    ".vert",
    VK_SHADER_STAGE_VERTEX_BIT,
    ShaderKind::Vertex,
    EShLangVertex,
};

const ShaderKindInfo ShaderKindInfo::tesc {
    ".tesc",
    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    ShaderKind::TessellationControl,
    EShLangTessControl,
};

const ShaderKindInfo ShaderKindInfo::tese {
    ".tese",
    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    ShaderKind::TessellationEvaluation,
    EShLangTessEvaluation,
};

const ShaderKindInfo ShaderKindInfo::geom {
    ".geom",
    VK_SHADER_STAGE_GEOMETRY_BIT,
    ShaderKind::Geometry,
    EShLangGeometry,
};

const ShaderKindInfo ShaderKindInfo::frag {
    ".frag",
    VK_SHADER_STAGE_FRAGMENT_BIT,
    ShaderKind::Fragment,
    EShLangFragment,
};

const ShaderKindInfo ShaderKindInfo::comp {
    ".comp",
    VK_SHADER_STAGE_COMPUTE_BIT,
    ShaderKind::Compute,
    EShLangCompute,
};


const std::array<ShaderKindInfo, 6> ShaderKindInfo::allShaderKinds ({
    ShaderKindInfo::vert,
    ShaderKindInfo::tese,
    ShaderKindInfo::tesc,
    ShaderKindInfo::geom,
    ShaderKindInfo::frag,
    ShaderKindInfo::comp,
});


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

const ShaderKindInfo ShaderKindInfo::FromShaderKind (ShaderKind sk)
{
    return Find ([&] (auto& s) {
        return s.shaderKind == sk;
    });
}

const ShaderKindInfo ShaderKindInfo::FromEsh (EShLanguage e)
{
    return Find ([&] (auto& s) {
        return s.esh == e;
    });
}


class ShaderCache {
private:
    const std::filesystem::path tempFolder;

public:
    ShaderCache ()
        : tempFolder (std::filesystem::temp_directory_path () / "GearsVk" / "ShaderCache")
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

        if (GVK_ERROR (!std::filesystem::exists (tempFolder / cachedBinaryFileName))) {
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

} shaderCache;


static std::vector<uint32_t> CompileWithGlslangCppInterface (const std::string& sourceCode, const ShaderKindInfo& shaderKind)
{
    using namespace glslang;

    static bool init = false;
    if (!init) {
        init = true;
        InitializeProcess ();
    }

    const char* const              sourceCstr                  = sourceCode.c_str ();
    const int                      ClientInputSemanticsVersion = 100;
    const EShTargetClientVersion   VulkanClientVersion         = EShTargetVulkan_1_0;
    const EShTargetLanguageVersion TargetVersion               = EShTargetSpv_1_0;
    const EShMessages              messages                    = (EShMessages) (EShMsgSpvRules | EShMsgVulkanRules);
    const TBuiltInResource         resources                   = DefaultResourceLimits; // TODO use DefaultTBuiltInResource ?


    TShader shader (shaderKind.esh);
    shader.setStrings (&sourceCstr, 1);
    shader.setEnvInput (EShSourceGlsl, shaderKind.esh, EShClientVulkan, ClientInputSemanticsVersion);
    shader.setEnvClient (EShClientVulkan, VulkanClientVersion);
    shader.setEnvTarget (EShTargetSpv, TargetVersion);

    if (!shader.parse (&resources, 100, false, messages)) {
        throw ShaderCompileException (shader.getInfoLog ());
    }
    TProgram program;
    program.addShader (&shader);

    if (!program.link (EShMsgDefault)) {
        throw ShaderCompileException (program.getInfoLog ());
    }

    spv::SpvBuildLogger logger;
    glslang::SpvOptions spvOptions;

    std::vector<uint32_t> spirvBinary;
    glslang::GlslangToSpv (*program.getIntermediate (shaderKind.esh), spirvBinary, &logger, &spvOptions);
    return spirvBinary;
}


static std::vector<uint32_t> CompileFromSourceCode (const std::string& shaderSource_, const ShaderKindInfo& shaderKind, ShaderPreprocessor& preprocessor)
{
    const std::string shaderSource = preprocessor.Preprocess (shaderSource_);

    std::optional<std::vector<uint32_t>> cachedBinary = shaderCache.Load (shaderSource);
    if (cachedBinary.has_value ()) {
        return *cachedBinary;
    }

    try {
        const std::vector<uint32_t> result = CompileWithGlslangCppInterface (shaderSource, shaderKind);
        shaderCache.Save (shaderSource, result);
        return result;

    } catch (ShaderCompileException& ex) {
        std::cout << ex.what () << std::endl;
        throw;
    }
}


static VkShaderModule CreateShaderModule (VkDevice device, const std::vector<uint32_t>& binary)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize                 = static_cast<uint32_t> (binary.size () * sizeof (uint32_t)); // size must be in bytes
    createInfo.pCode                    = reinterpret_cast<const uint32_t*> (binary.data ());

    VkShaderModule result = VK_NULL_HANDLE;
    if (GVK_ERROR (vkCreateShaderModule (device, &createInfo, nullptr, &result) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create shader module");
    }

    return result;
}


static VkShaderModule CreateShaderModule (VkDevice device, const std::vector<char>& binary)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize                 = binary.size ();
    createInfo.pCode                    = reinterpret_cast<const uint32_t*> (binary.data ());

    VkShaderModule result = VK_NULL_HANDLE;
    if (GVK_ERROR (vkCreateShaderModule (device, &createInfo, nullptr, &result) != VK_SUCCESS)) {
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
                            const std::string&           sourceCode,
                            ShaderPreprocessor&          preprocessor)
    : readMode (readMode)
    , shaderKind (shaderKind)
    , device (device)
    , handle (handle)
    , fileLocation (fileLocation)
    , binary (binary)
    , reflection (binary)
    , sourceCode (sourceCode)
    , preprocessor (preprocessor)
{
}


ShaderModuleU ShaderModule::CreateFromSPVFile (VkDevice device, ShaderKind shaderKind, const std::filesystem::path& fileLocation)
{
    std::optional<std::vector<char>> binaryC = Utils::ReadBinaryFile (fileLocation);
    if (GVK_ERROR (!binaryC.has_value ())) {
        throw std::runtime_error ("failed to read shader");
    }

    std::vector<uint32_t> code;
    code.resize (binaryC->size () / sizeof (uint32_t));
    memcpy (code.data (), binaryC->data (), binaryC->size ());

    VkShaderModule handle = CreateShaderModule (device, *binaryC);

    return ShaderModule::Create (shaderKind, ReadMode::SPVFilePath, device, handle, fileLocation, code, "", emptyPreprocessor);
}


ShaderModuleU ShaderModule::CreateFromGLSLFile (VkDevice device, const std::filesystem::path& fileLocation, ShaderPreprocessor& preprocessor)
{
    std::optional<std::string> fileContents = Utils::ReadTextFile (fileLocation);
    if (GVK_ERROR (!fileContents.has_value ())) {
        throw std::runtime_error ("failed to read shader");
    }

    std::optional<std::vector<uint32_t>> binary = CompileFromSourceCode (*fileContents, ShaderKindInfo::FromExtension (fileLocation.extension ().u8string ()), preprocessor);
    if (GVK_ERROR (!binary.has_value ())) {
        throw std::runtime_error ("failed to compile shader");
    }

    VkShaderModule handle = CreateShaderModule (device, *binary);

    return ShaderModule::Create (
        ShaderKindInfo::FromExtension (fileLocation.extension ().u8string ()).shaderKind,
        ReadMode::GLSLFilePath,
        device,
        handle,
        fileLocation,
        *binary,
        *fileContents,
        preprocessor);
}


ShaderModuleU ShaderModule::CreateFromGLSLString (VkDevice device, ShaderKind shaderKind, const std::string& shaderSource, ShaderPreprocessor& preprocessor)
{
    std::vector<uint32_t> binary = CompileFromSourceCode (shaderSource, ShaderKindInfo::FromShaderKind (shaderKind), preprocessor);

    VkShaderModule handle = CreateShaderModule (device, binary);

    return ShaderModule::Create (
        shaderKind,
        ReadMode::GLSLString,
        device,
        handle,
        "",
        binary,
        shaderSource,
        preprocessor);
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


ShaderModule::Reflection::Reflection (const std::vector<uint32_t>& binary)
    : ubos (SR::GetUBOsFromBinary (binary))
    , samplers (SR::GetSamplersFromBinary (binary))
    , inputs (SR::GetInputsFromBinary (binary))
    , outputs (SR::GetOutputsFromBinary (binary))
{
}


void ShaderModule::Reload ()
{
    if (readMode == ReadMode::GLSLFilePath) {
        vkDestroyShaderModule (device, handle, nullptr);

        std::optional<std::string> fileContents = Utils::ReadTextFile (fileLocation);
        if (GVK_ERROR (!fileContents.has_value ())) {
            throw std::runtime_error ("failed to read shader");
        }

        std::optional<std::vector<uint32_t>> newBinary = CompileFromSourceCode (*fileContents, ShaderKindInfo::FromExtension (fileLocation.extension ().u8string ()), preprocessor);
        if (GVK_ERROR (!newBinary.has_value ())) {
            throw std::runtime_error ("failed to compile shader");
        }

        handle = CreateShaderModule (device, *newBinary);

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

        handle = CreateShaderModule (device, *binaryC);

        reflection = Reflection (binary);

        binary = code;

    } else if (readMode == ReadMode::GLSLString) {
        GVK_BREAK ("cannot reload shaders from hard coded strings");

    } else {
        GVK_BREAK ("unknown read mode");
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
            GVK_BREAK ("unexpected shaderkind type");
            return "";
    }
}
