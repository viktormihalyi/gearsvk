#ifndef SHADERMODULE_HPP
#define SHADERMODULE_HPP

#include "GearsVkAPI.hpp"

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "ShaderReflection.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

#include <filesystem>

#include <vulkan/vulkan.h>

class GEARSVK_API ShaderCompileException : public std::runtime_error {
public:
    ShaderCompileException (const std::string& errorMessage)
        : std::runtime_error (errorMessage)
    {
    }
};

enum class ShaderKind : uint8_t {
    Vertex,
    Fragment,
    TessellationControl,
    TessellationEvaluation,
    Geometry,
    Compute,
};


std::string ShaderKindToString (ShaderKind);


USING_PTR (ShaderPreprocessor);
class GEARSVK_API ShaderPreprocessor {
public:
    virtual ~ShaderPreprocessor () = default;

    virtual std::string Preprocess (const std::string& source) = 0;
};


class GEARSVK_API EmptyPreprocessor : public ShaderPreprocessor {
public:
    virtual std::string Preprocess (const std::string& source) override
    {
        return source;
    }
};

extern GEARSVK_API EmptyPreprocessor emptyPreprocessor;


USING_PTR (ShaderModule);
class GEARSVK_API ShaderModule : public VulkanObject {
    USING_CREATE (ShaderModule);

public:
    static constexpr uint32_t ShaderKindCount = 6;


    enum class ReadMode {
        GLSLFilePath,
        SPVFilePath,
        GLSLString,
    };

    struct Reflection {
        ShaderKind shaderKind;

        std::vector<SR::UBOP>    ubos;
        std::vector<SR::Sampler> samplers;
        std::vector<SR::Input>   inputs;
        std::vector<SR::Output>  outputs;

        Reflection (ShaderKind shaderKind, const std::vector<uint32_t>& binary);

        std::vector<VkWriteDescriptorSet> GetDescriptorWrites (
            VkDescriptorSet                                                            dstSet,
            const std::function<VkDescriptorImageInfo (const SR::Sampler&, uint32_t)>& imageInfoProvider,
            const std::function<VkDescriptorBufferInfo (const SR::UBO&)>&              bufferInfoProvider) const;

        std::vector<VkDescriptorSetLayoutBinding> GetLayout () const;

        struct Buff {
            std::set<std::string> attributes;
            bool                  instanced;
        };

        std::vector<VkVertexInputAttributeDescription> GetVertexAttributes (const std::function<bool (const std::string&)>& instanceNameProvider) const;
        std::vector<VkVertexInputBindingDescription>   GetVertexBindings (const std::function<bool (const std::string&)>& instanceNameProvider) const;
    };

private:
    const VkDevice   device;
    VkShaderModule   handle;
    const ReadMode   readMode;
    const ShaderKind shaderKind;

    std::string                 sourceCode;
    const std::filesystem::path fileLocation;
    std::vector<uint32_t>       binary;

    Reflection reflection;

    ShaderPreprocessor& preprocessor;

private:
    // private ctor, use factories instead
    ShaderModule (ShaderKind                   shaderKind,
                  ReadMode                     mode,
                  VkDevice                     device,
                  VkShaderModule               handle,
                  const std::filesystem::path& fileLocation,
                  const std::vector<uint32_t>& binary,
                  const std::string&           sourceCode,
                  ShaderPreprocessor&          preprocessor);

public:
    static ShaderModuleU CreateFromGLSLString (VkDevice device, ShaderKind shaderKind, const std::string& shaderSource, ShaderPreprocessor& preprocessor = emptyPreprocessor);
    static ShaderModuleU CreateFromGLSLFile (VkDevice device, const std::filesystem::path& fileLocation, ShaderPreprocessor& preprocessor = emptyPreprocessor);
    static ShaderModuleU CreateFromSPVFile (VkDevice device, ShaderKind shaderKind, const std::filesystem::path& fileLocation);

    virtual ~ShaderModule ();

    void Reload ();

    const std::vector<uint32_t>& GetBinary () const { return binary; }

    const std::filesystem::path& GetLocation () const { return fileLocation; }

    ShaderKind GetShaderKind () const { return shaderKind; }

    ReadMode GetReadMode () const { return readMode; }

    VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo () const;

    const Reflection& GetReflection () const { return reflection; }
};

#endif