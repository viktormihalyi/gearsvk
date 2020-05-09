#include "ShaderModule.hpp"
#include "Assert.hpp"
#include "ShaderReflection.hpp"
#include "Utils.hpp"

#include <array>
#include <iostream>


#include <SPIRV/GlslangToSpv.h>
#include <glslang/MachineIndependent/reflection.h>
#include <glslang/Public/ShaderLang.h>

#include "ResourceLimits.hpp"

class ShaderKindInfo final {
public:
    const std::string              extension;
    const VkShaderStageFlagBits    vkflag;
    const ShaderModule::ShaderKind shaderKind;
    const EShLanguage              esh;

private:
    ShaderKindInfo () = default;

public:
    static const ShaderKindInfo FromExtension (const std::string&);
    static const ShaderKindInfo FromVk (VkShaderStageFlagBits);
    static const ShaderKindInfo FromShaderKind (ShaderModule::ShaderKind);
    static const ShaderKindInfo FromEsh (EShLanguage);

private:
    static const ShaderKindInfo Find (const std::function<bool (const ShaderKindInfo&)>&);
    static const ShaderKindInfo vert, tesc, tese, geom, frag, comp;

    static const std::array<ShaderKindInfo, 6> allShaderKinds;
};

const ShaderKindInfo ShaderKindInfo::vert {
    ".vert",
    VK_SHADER_STAGE_VERTEX_BIT,
    ShaderModule::ShaderKind::Vertex,
    EShLangVertex,
};

const ShaderKindInfo ShaderKindInfo::tesc {
    ".tesc",
    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    ShaderModule::ShaderKind::TessellationControl,
    EShLangTessControl,
};

const ShaderKindInfo ShaderKindInfo::tese {
    ".tese",
    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    ShaderModule::ShaderKind::TessellationEvaluation,
    EShLangTessEvaluation,
};

const ShaderKindInfo ShaderKindInfo::geom {
    ".geom",
    VK_SHADER_STAGE_GEOMETRY_BIT,
    ShaderModule::ShaderKind::Geometry,
    EShLangGeometry,
};

const ShaderKindInfo ShaderKindInfo::frag {
    ".frag",
    VK_SHADER_STAGE_FRAGMENT_BIT,
    ShaderModule::ShaderKind::Fragment,
    EShLangFragment,
};

const ShaderKindInfo ShaderKindInfo::comp {
    ".comp",
    VK_SHADER_STAGE_COMPUTE_BIT,
    ShaderModule::ShaderKind::Compute,
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

const ShaderKindInfo ShaderKindInfo::FromShaderKind (ShaderModule::ShaderKind sk)
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


static uint32_t GetBasicTypeSize (glslang::TBasicType basicType)
{
    using namespace glslang;

    switch (basicType) {
        case EbtFloat: return sizeof (float);
        case EbtDouble: return sizeof (double);
        case EbtFloat16: ASSERT ("WTF"); return 2;
        case EbtInt8: return sizeof (int8_t);
        case EbtUint8: return sizeof (uint8_t);
        case EbtInt16: return sizeof (int16_t);
        case EbtUint16: return sizeof (uint16_t);
        case EbtInt: return sizeof (int);
        case EbtUint: return sizeof (unsigned int);
        case EbtInt64: return sizeof (int64_t);
        case EbtUint64: return sizeof (uint64_t);
        case EbtBool: return sizeof (bool);
        default: return 0;
    }
}


static std::optional<uint32_t> GetSize (const glslang::TType& type)
{
    using namespace glslang;

    const TBasicType basicType = type.getBasicType ();

    const uint32_t basicTypeSize = GetBasicTypeSize (basicType);
    if (basicTypeSize == 0) {
        return std::nullopt;
    }

    const uint32_t vectorSize = type.getVectorSize ();
    const uint32_t matrixCols = type.getMatrixCols ();
    const uint32_t matrixRows = type.getMatrixRows ();

    if (vectorSize > 0) {
        return basicTypeSize * vectorSize;
    }

    if (matrixCols > 0 && matrixRows > 0) {
        return basicTypeSize * matrixCols * matrixRows;
    }

    ASSERT ("unhandled uniform size case");
    return std::nullopt;
}

static std::optional<SR::Sampler::Type> GetSamplerType (glslang::TSamplerDim glslangType)
{
    using namespace glslang;

    switch (glslangType) {
        case Esd1D: return SR::Sampler::Type::Sampler1D;
        case Esd2D: return SR::Sampler::Type::Sampler2D;
        case Esd3D: return SR::Sampler::Type::Sampler3D;
        case EsdCube: return SR::Sampler::Type::SamplerCube;
        default: return std::nullopt;
    }
}


static std::vector<SR::Sampler> GetSamplers (glslang::TReflection& ref)
{
    using namespace glslang;

    std::vector<SR::Sampler> result;
    const uint32_t           uniformCount = ref.getNumUniforms ();
    for (uint32_t uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex) {
        const TObjectReflection& uniform = ref.getUniform (uniformIndex);
        if (uniform.getType ()) {
            const TType& type = *uniform.getType ();
            if (type.getBasicType () == EbtSampler) {
                SR::Sampler s;
                s.name = uniform.name;

                const std::optional<SR::Sampler::Type> stype = GetSamplerType (type.getSampler ().dim);
                if (ERROR (!stype.has_value ())) {
                    continue;
                }
                s.type = *stype;

                s.binding = type.getQualifier ().layoutBinding;
                result.push_back (s);
            }
        }
    }
    return result;
}


static std::vector<SR::UBO> GetUniformBlocks (glslang::TReflection& ref)
{
    using namespace glslang;

    std::vector<SR::UBO> result;

    for (uint32_t uniformIndex = 0; uniformIndex < ref.getNumUniformBlocks (); ++uniformIndex) {
        const TObjectReflection& uniform = ref.getUniformBlock (uniformIndex);
        if (uniform.getType ()) {
            const TType&      type      = *uniform.getType ();
            const TQualifier& qualifier = type.getQualifier ();

            if (type.isStruct ()) {
                SR::UBO ubo;
                ubo.name    = type.getTypeName ();
                ubo.binding = qualifier.layoutBinding;

                const TTypeList& structure = *type.getStruct ();
                for (auto& s : structure) {
                    if (ASSERT (s.type != nullptr)) {
                        SR::UBO::Field f;
                        f.name   = s.type->getFieldName ();
                        f.offset = s.type->getQualifier ().layoutOffset;
                        if (f.offset == UINT32_MAX) {
                            f.offset = 0;
                        }

                        const std::optional<uint32_t> size = GetSize (*s.type);
                        ASSERT (size.has_value ());
                        f.size = size.value_or (0);

                        ubo.fields.push_back (f);
                    }
                }
                result.push_back (ubo);
            }
        }
    }
    return result;
}


static std::vector<uint32_t> CompileWithGlslangCppInterface (const std::string& sourceCode, const ShaderKindInfo& shaderKind, ShaderModule::Reflection& reflection)
{
    using namespace glslang;

    static bool init = false;
    if (!init) {
        init = true;
        InitializeProcess ();
    }

    reflection.Clear ();

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

    TReflection ref (EShReflectionAllBlockVariables, shaderKind.esh, shaderKind.esh);
    ref.addStage (shaderKind.esh, *shader.getIntermediate ());

    reflection.samplers = GetSamplers (ref);
    reflection.ubos     = GetUniformBlocks (ref);

    spv::SpvBuildLogger logger;
    glslang::SpvOptions spvOptions;

    std::vector<uint32_t> spirvBinary;
    glslang::GlslangToSpv (*program.getIntermediate (shaderKind.esh), spirvBinary, &logger, &spvOptions);
    return spirvBinary;
}


static std::vector<uint32_t> CompileFromSourceCode (const std::string& shaderSource, const ShaderKindInfo& shaderKind, ShaderModule::Reflection& reflection)
{
    try {
        return CompileWithGlslangCppInterface (shaderSource, shaderKind, reflection);
    } catch (ShaderCompileException& ex) {
        std::cout << ex.what () << std::endl;
        throw;
    }
}


static std::optional<std::vector<uint32_t>> CompileShaderFromFile (const std::filesystem::path& fileLocation, ShaderModule::Reflection& reflection)
{
    std::optional<std::string> fileContents = Utils::ReadTextFile (fileLocation);
    if (ERROR (!fileContents.has_value ())) {
        return std::nullopt;
    }

    return CompileFromSourceCode (*fileContents, ShaderKindInfo::FromExtension (fileLocation.extension ().u8string ()), reflection);
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


ShaderModule::ShaderModule (ShaderModule::ShaderKind shaderKind, ReadMode readMode, VkDevice device, VkShaderModule handle, const std::filesystem::path& fileLocation, const std::vector<uint32_t>& binary)
    : readMode (readMode)
    , shaderKind (shaderKind)
    , device (device)
    , handle (handle)
    , fileLocation (fileLocation)
    , binary (binary)
{
}


ShaderModule::U ShaderModule::CreateFromSPVFile (VkDevice device, const std::filesystem::path& fileLocation)
{
    std::optional<std::vector<char>>     binaryC = Utils::ReadBinaryFile (fileLocation);
    std::optional<std::vector<uint32_t>> binary  = Utils::ReadBinaryFile4Byte (fileLocation);
    if (ERROR (!binary.has_value ())) {
        throw std::runtime_error ("failed to read shader");
    }

    VkShaderModule handle = CreateShaderModule (device, *binary);

    return ShaderModule::Create (ShaderKindInfo::FromExtension (fileLocation.extension ().u8string ()).shaderKind, ReadMode::SPVFilePath, device, handle, fileLocation, *binary);
}


ShaderModule::U ShaderModule::CreateFromGLSLFile (VkDevice device, const std::filesystem::path& fileLocation)
{
    ShaderModule::Reflection             reflection;
    std::optional<std::vector<uint32_t>> binary = CompileShaderFromFile (fileLocation, reflection);
    if (ERROR (!binary.has_value ())) {
        throw std::runtime_error ("failed to compile shader");
    }

    VkShaderModule handle = CreateShaderModule (device, *binary);

    auto sm        = ShaderModule::Create (ShaderKindInfo::FromExtension (fileLocation.extension ().u8string ()).shaderKind, ReadMode::GLSLFilePath, device, handle, fileLocation, *binary);
    sm->reflection = reflection;
    return sm;
}

ShaderModule::U ShaderModule::CreateFromGLSLString (VkDevice device, ShaderKind shaderKind, const std::string& shaderSource)
{
    ShaderModule::Reflection reflection;
    std::vector<uint32_t>    binary = CompileFromSourceCode (shaderSource, ShaderKindInfo::FromShaderKind (shaderKind), reflection);

    VkShaderModule handle = CreateShaderModule (device, binary);

    auto sm        = ShaderModule::Create (shaderKind, ReadMode::GLSLString, device, handle, "", binary);
    sm->reflection = reflection;
    return sm;
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
    if (readMode == ReadMode::GLSLFilePath) {
        vkDestroyShaderModule (device, handle, nullptr);

        std::optional<std::vector<uint32_t>> binary = CompileShaderFromFile (fileLocation, reflection);
        if (ERROR (!binary.has_value ())) {
            throw std::runtime_error ("failed to compile shader");
        }

        VkShaderModule handle = CreateShaderModule (device, *binary);

    } else {
        ASSERT ("unimplemented");
    }
}
