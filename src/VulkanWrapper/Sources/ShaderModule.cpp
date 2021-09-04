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
#include <SPIRV/GlslangToSpv.h>

// from spdlog
#include "spdlog/spdlog.h"


static Utils::CommandLineOnOffFlag disableShaderCacheFlag ("--disableShaderCache");


namespace GVK {


class ShaderKindAdapter final {
public:
    const char*           extension;
    VkShaderStageFlagBits vkflag;
    ShaderKind            shaderKind;
    EShLanguage           esh;
    const char*           displayName;

private:
    ShaderKindAdapter (const char*           extension,
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

public:
    static const ShaderKindAdapter FromExtension (const std::string&);
    static const ShaderKindAdapter FromVk (VkShaderStageFlagBits);
    static const ShaderKindAdapter FromShaderKind (ShaderKind);
    static const ShaderKindAdapter FromEsh (EShLanguage);

private:
    static const ShaderKindAdapter Find (const std::function<bool (const ShaderKindAdapter&)>&);
    static const ShaderKindAdapter vert, tesc, tese, geom, frag, comp;

    static const std::array<ShaderKindAdapter, 6> allShaderKinds;
};

const ShaderKindAdapter ShaderKindAdapter::vert {
    ".vert",
    VK_SHADER_STAGE_VERTEX_BIT,
    ShaderKind::Vertex,
    EShLangVertex,
    "Vertex Shader"
};

const ShaderKindAdapter ShaderKindAdapter::tesc {
    ".tesc",
    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    ShaderKind::TessellationControl,
    EShLangTessControl,
    "Tessellation Control Shader"
};

const ShaderKindAdapter ShaderKindAdapter::tese {
    ".tese",
    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    ShaderKind::TessellationEvaluation,
    EShLangTessEvaluation,
    "Tessellation Evaluation Shader"
};

const ShaderKindAdapter ShaderKindAdapter::geom {
    ".geom",
    VK_SHADER_STAGE_GEOMETRY_BIT,
    ShaderKind::Geometry,
    EShLangGeometry,
    "Geometry Shader"
};

const ShaderKindAdapter ShaderKindAdapter::frag {
    ".frag",
    VK_SHADER_STAGE_FRAGMENT_BIT,
    ShaderKind::Fragment,
    EShLangFragment,
    "Fragment Shader"
};

const ShaderKindAdapter ShaderKindAdapter::comp {
    ".comp",
    VK_SHADER_STAGE_COMPUTE_BIT,
    ShaderKind::Compute,
    EShLangCompute,
    "Compute Shader"
};


const std::array<ShaderKindAdapter, 6> ShaderKindAdapter::allShaderKinds ({
    ShaderKindAdapter::vert,
    ShaderKindAdapter::tese,
    ShaderKindAdapter::tesc,
    ShaderKindAdapter::geom,
    ShaderKindAdapter::frag,
    ShaderKindAdapter::comp,
});


const ShaderKindAdapter ShaderKindAdapter::Find (const std::function<bool (const ShaderKindAdapter&)>& callback)
{
    for (auto& s : allShaderKinds) {
        if (callback (s)) {
            return s;
        }
    }
    throw std::runtime_error ("couldnt find shader info");
}

const ShaderKindAdapter ShaderKindAdapter::FromExtension (const std::string& ext)
{
    return Find ([&] (auto& s) {
        return s.extension == ext;
    });
}

const ShaderKindAdapter ShaderKindAdapter::FromVk (VkShaderStageFlagBits flag)
{
    return Find ([&] (auto& s) {
        return s.vkflag == flag;
    });
}

const ShaderKindAdapter ShaderKindAdapter::FromShaderKind (ShaderKind sk)
{
    return Find ([&] (auto& s) {
        return s.shaderKind == sk;
    });
}

const ShaderKindAdapter ShaderKindAdapter::FromEsh (EShLanguage e)
{
    return Find ([&] (auto& s) {
        return s.esh == e;
    });
}


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


static std::vector<uint32_t> CompileWithGlslangCppInterface (const std::string& sourceCode, const ShaderKindAdapter& shaderKind)
{
    static bool init = false;
    if (!init) {
        init = true;
        glslang::InitializeProcess ();
    }

    const char* const                       sourceCstr                  = sourceCode.c_str ();
    const int                               ClientInputSemanticsVersion = 100;
    const glslang::EShTargetClientVersion   VulkanClientVersion         = glslang::EShTargetVulkan_1_0;
    const glslang::EShTargetLanguageVersion TargetVersion               = glslang::EShTargetSpv_1_0;
    const EShMessages                       messages                    = (EShMessages) (EShMsgSpvRules | EShMsgVulkanRules);
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
    spvOptions.disableOptimizer = false;
    spvOptions.validate         = true;

    std::vector<uint32_t> spirvBinary;
    glslang::GlslangToSpv (*program.getIntermediate (shaderKind.esh), spirvBinary, &logger, &spvOptions);

    const std::string loggerMessages = logger.getAllMessages ();
    if (GVK_ERROR (!loggerMessages.empty ()))
        spdlog::error ("{}", loggerMessages);

    return spirvBinary;
}


static std::vector<uint32_t> CompileFromSourceCode (const std::string& shaderSource, const ShaderKindAdapter& shaderKind)
{
    ShaderCache& shaderCache = IsDebugBuild ? debugShaderCache : releaseShaderCache;

    std::optional<std::vector<uint32_t>> cachedBinary = shaderCache.Load (shaderSource);
    if (cachedBinary.has_value ()) {
        return *cachedBinary;
    }

    try {
        const std::vector<uint32_t> result = CompileWithGlslangCppInterface (shaderSource, shaderKind);
        shaderCache.Save (shaderSource, result);
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
    , reflection (shaderKind, binary)
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
        throw std::runtime_error ("failed to read shader");
    }

    std::optional<std::vector<uint32_t>> binary = CompileFromSourceCode (*fileContents, ShaderKindAdapter::FromExtension (fileLocation.extension ().string ()));
    if (GVK_ERROR (!binary.has_value ())) {
        throw std::runtime_error ("failed to compile shader");
    }

    VkShaderModule handle = CreateShaderModuleImpl (device, *binary);
    
    return std::unique_ptr<ShaderModule> (new ShaderModule (
        ShaderKindAdapter::FromExtension (fileLocation.extension ().string ()).shaderKind,
        ReadMode::GLSLFilePath,
        device,
        handle,
        fileLocation,
        *binary,
        *fileContents));
}


std::unique_ptr<ShaderModule> ShaderModule::CreateFromGLSLString (VkDevice device, ShaderKind shaderKind, const std::string& shaderSource)
{
    std::vector<uint32_t> binary = CompileFromSourceCode (shaderSource, ShaderKindAdapter::FromShaderKind (shaderKind));

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
    result.stage                           = ShaderKindAdapter::FromShaderKind (shaderKind).vkflag;
    result.module                          = handle;
    result.pName                           = "main";
    return result;
}


ShaderModule::Reflection::Reflection (ShaderKind shaderKind, const std::vector<uint32_t>& binary)
    : shaderKind (shaderKind)
{
    SR::ReflCompiler c (binary);

    ubos          = SR::GetUBOsFromBinary (c);
    samplers      = SR::GetSamplersFromBinary (c);
    inputs        = SR::GetInputsFromBinary (c);
    outputs       = SR::GetOutputsFromBinary (c);
    subpassInputs = SR::GetSubpassInputsFromBinary (c);
}


static VkShaderStageFlags GetShaderStageFromShaderKind (ShaderKind shaderKind)
{
    switch (shaderKind) {
        case ShaderKind::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderKind::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderKind::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderKind::TessellationControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderKind::TessellationEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderKind::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
        default: GVK_BREAK_STR ("unexpected shaderkind type"); return VK_SHADER_STAGE_ALL;
    }
}


std::vector<VkDescriptorSetLayoutBinding>
ShaderModule::Reflection::GetLayout () const
{
    std::vector<VkDescriptorSetLayoutBinding> result;

    for (const SR::Sampler& sampler : samplers) {
        VkDescriptorSetLayoutBinding bin = {};
        bin.binding                      = sampler.binding;
        bin.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bin.descriptorCount              = sampler.arraySize == 0 ? 1 : sampler.arraySize;
        bin.stageFlags                   = GetShaderStageFromShaderKind (shaderKind);
        bin.pImmutableSamplers           = nullptr;
        result.push_back (bin);
    }

    for (const std::shared_ptr<SR::UBO>& ubo : ubos) {
        VkDescriptorSetLayoutBinding bin = {};
        bin.binding                      = ubo->binding;
        bin.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bin.descriptorCount              = 1;
        bin.stageFlags                   = GetShaderStageFromShaderKind (shaderKind);
        bin.pImmutableSamplers           = nullptr;
        result.push_back (bin);
    }

    for (const SR::SubpassInput& subpassInput : subpassInputs) {
        VkDescriptorSetLayoutBinding bin = {};
        bin.binding                      = subpassInput.binding;
        bin.descriptorType               = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        bin.descriptorCount              = 1;
        bin.stageFlags                   = GetShaderStageFromShaderKind (shaderKind);
        bin.pImmutableSamplers           = nullptr;
        result.push_back (bin);
    }

    return result;
}


std::vector<VkDescriptorImageInfo> ShaderModule::Reflection::DescriptorWriteInfoTable::GetDescriptorImageInfos (const std::string& name, ShaderKind shaderKind, uint32_t layerIndex, uint32_t frameIndex)
{
    std::vector<DescriptorImageInfoTableEntry> result = imageInfos;
    result.erase (std::remove_if (result.begin (), result.end (), [&] (const auto& entry) {
                      return entry.name != name || entry.shaderKind != shaderKind;
                  }),
                  result.end ());

    std::vector<VkDescriptorImageInfo> resultInfos;

    for (const auto& entry : result)
        resultInfos.push_back ({ entry.sampler (), entry.imageView (frameIndex, layerIndex), entry.imageLayout });

    return resultInfos;
}


std::vector<VkDescriptorBufferInfo> ShaderModule::Reflection::DescriptorWriteInfoTable::GetDescriptorBufferInfos (const std::string& name, ShaderKind shaderKind, uint32_t frameIndex)
{
    std::vector<DescriptorBufferInfoTableEntry> result = bufferInfos;
    result.erase (std::remove_if (result.begin (), result.end (), [&] (const auto& entry) {
                      return entry.name != name || entry.shaderKind != shaderKind;
                  }),
                  result.end ());

    std::vector<VkDescriptorBufferInfo> resultInfos;

    for (const auto& entry : result)
        resultInfos.push_back ({ entry.buffer (frameIndex), entry.offset, entry.range });

    return resultInfos;
}


size_t ShaderModule::Reflection::WriteDescriptors (VkDevice device, VkDescriptorSet dstSet, uint32_t frameIndex, ShaderKind shaderKind, IDescriptorWriteInfoProvider& infoProvider) const
{
    std::vector<VkDescriptorImageInfo>  imgInfos;
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkWriteDescriptorSet>   result;

    imgInfos.reserve (1024);
    bufferInfos.reserve (1024);
    result.reserve (1024);

    for (const SR::Sampler& sampler : samplers) {
        const uint32_t layerCount = sampler.arraySize == 0 ? 1 : sampler.arraySize;
        for (uint32_t layerIndex = 0; layerIndex < layerCount; ++layerIndex) {
            const std::vector<VkDescriptorImageInfo> tempImgInfos = infoProvider.GetDescriptorImageInfos (sampler.name, shaderKind, layerIndex, frameIndex);
            if (GVK_ERROR (tempImgInfos.empty ()))
                continue;

            const int32_t currentSize = imgInfos.size ();

            imgInfos.insert (imgInfos.end (), tempImgInfos.begin (), tempImgInfos.end ());

            const int32_t newSize = imgInfos.size ();

            GVK_ASSERT (newSize - currentSize == tempImgInfos.size ());
            GVK_ASSERT (newSize - currentSize > 0);

            VkWriteDescriptorSet write = {};
            write.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet               = dstSet;
            write.dstBinding           = sampler.binding;
            write.dstArrayElement      = layerIndex;
            write.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.descriptorCount      = newSize - currentSize;
            write.pBufferInfo          = nullptr;
            write.pImageInfo           = &imgInfos[currentSize];
            write.pTexelBufferView     = nullptr;

            result.push_back (write);
            }
    }

    if (device != VK_NULL_HANDLE && !result.empty ())
        vkUpdateDescriptorSets (device, result.size (), result.data (), 0, nullptr);
    result.clear ();
    imgInfos.clear ();
    bufferInfos.clear ();

    for (const std::shared_ptr<SR::UBO>& ubo : ubos) {
        const std::vector<VkDescriptorBufferInfo> tempBufferInfos = infoProvider.GetDescriptorBufferInfos (ubo->name, shaderKind, frameIndex);
        if (GVK_ERROR (tempBufferInfos.empty ()))
            continue;

        const int32_t currentSize = bufferInfos.size ();

        bufferInfos.insert (bufferInfos.end (), tempBufferInfos.begin (), tempBufferInfos.end ());

        const int32_t newSize = bufferInfos.size ();

        GVK_ASSERT (newSize - currentSize == tempBufferInfos.size ());
        GVK_ASSERT (newSize - currentSize > 0);

        VkWriteDescriptorSet write = {};
        write.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet               = dstSet;
        write.dstBinding           = ubo->binding;
        write.dstArrayElement      = 0;
        write.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount      = newSize - currentSize;
        write.pBufferInfo          = &bufferInfos[currentSize];
        write.pImageInfo           = nullptr;
        write.pTexelBufferView     = nullptr;

        result.push_back (write);
    }

    if (device != VK_NULL_HANDLE && !result.empty ())
        vkUpdateDescriptorSets (device, result.size (), result.data (), 0, nullptr);
    result.clear ();
    imgInfos.clear ();
    bufferInfos.clear ();

    for (const SR::SubpassInput& subpassInput : subpassInputs) {
        const uint32_t layerCount = subpassInput.arraySize == 0 ? 1 : subpassInput.arraySize;
        for (uint32_t layerIndex = 0; layerIndex < layerCount; ++layerIndex) {
            const std::vector<VkDescriptorImageInfo> tempImgInfos = infoProvider.GetDescriptorImageInfos (subpassInput.name, shaderKind, layerIndex, frameIndex);
            if (GVK_ERROR (tempImgInfos.empty ()))
                continue;

            const int32_t currentSize = imgInfos.size ();

            imgInfos.insert (imgInfos.end (), tempImgInfos.begin (), tempImgInfos.end ());

            const int32_t newSize = imgInfos.size ();

            GVK_ASSERT (newSize - currentSize == tempImgInfos.size ());
            GVK_ASSERT (newSize - currentSize > 0);

            VkWriteDescriptorSet write = {};
            write.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet               = dstSet;
            write.dstBinding           = subpassInput.binding;
            write.dstArrayElement      = layerIndex;
            write.descriptorType       = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            write.descriptorCount      = newSize - currentSize;
            write.pBufferInfo          = nullptr;
            write.pImageInfo           = &imgInfos[currentSize];
            write.pTexelBufferView     = nullptr;

            result.push_back (write);
        }
    }

    if (device != VK_NULL_HANDLE && !result.empty ())
        vkUpdateDescriptorSets (device, result.size (), result.data (), 0, nullptr);
    result.clear ();
    imgInfos.clear ();
    bufferInfos.clear ();

    return result.size ();
}


std::vector<VkVertexInputAttributeDescription> ShaderModule::Reflection::GetVertexAttributes (const std::function<bool (const std::string&)>& IsInputInstanced) const
{
    GVK_ASSERT (shaderKind == ShaderKind::Vertex);

    std::vector<VkVertexInputAttributeDescription> result;

    uint32_t currentOffsetVertex   = 0;
    uint32_t currentOffsetInstance = 0;

    for (const SR::Input& input : inputs) {
        VkVertexInputAttributeDescription attrib = {};
        attrib.binding                           = 0;
        attrib.location                          = input.location;
        attrib.format                            = FieldTypeToVkFormat (input.type);

        if (IsInputInstanced (input.name)) {
            attrib.offset = currentOffsetInstance;
            currentOffsetInstance += input.sizeInBytes;
        } else {
            attrib.offset = currentOffsetVertex;
            currentOffsetVertex += input.sizeInBytes;
        }

        result.push_back (attrib);
    }

    return result;
}


std::vector<VkVertexInputBindingDescription> ShaderModule::Reflection::GetVertexBindings (const std::function<bool (const std::string&)>& IsInputInstanced) const
{
    GVK_ASSERT (shaderKind == ShaderKind::Vertex);

    if (inputs.empty ()) {
        return {};
    }

    std::vector<VkVertexInputBindingDescription> result;

    uint32_t fullSizeVertex   = 0;
    uint32_t fullSizeInstance = 0;

    for (const SR::Input& input : inputs) {
        if (IsInputInstanced (input.name)) {
            fullSizeInstance += input.sizeInBytes;
        } else {
            fullSizeVertex += input.sizeInBytes;
        }
    }

    {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription                                 = {};
        bindingDescription.binding                         = 0;
        bindingDescription.stride                          = fullSizeVertex;
        bindingDescription.inputRate                       = VK_VERTEX_INPUT_RATE_VERTEX;
        result.push_back (bindingDescription);
    }

    if (fullSizeInstance > 0) {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription                                 = {};
        bindingDescription.binding                         = 0;
        bindingDescription.stride                          = fullSizeInstance;
        bindingDescription.inputRate                       = VK_VERTEX_INPUT_RATE_INSTANCE;
        result.push_back (bindingDescription);
    }

    return result;
}


void ShaderModule::Reload ()
{
    if (readMode == ReadMode::GLSLFilePath) {
        vkDestroyShaderModule (device, handle, nullptr);

        std::optional<std::string> fileContents = Utils::ReadTextFile (fileLocation);
        if (GVK_ERROR (!fileContents.has_value ())) {
            throw std::runtime_error ("failed to read shader");
        }

        std::optional<std::vector<uint32_t>> newBinary = CompileFromSourceCode (*fileContents, ShaderKindAdapter::FromExtension (fileLocation.extension ().string ()));
        if (GVK_ERROR (!newBinary.has_value ())) {
            throw std::runtime_error ("failed to compile shader");
        }

        handle = CreateShaderModuleImpl (device, *newBinary);

        binary = *newBinary;

        reflection = Reflection (shaderKind, binary);

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

        reflection = Reflection (shaderKind, binary);

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