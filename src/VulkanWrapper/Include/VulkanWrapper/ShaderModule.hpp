#ifndef SHADERMODULE_HPP
#define SHADERMODULE_HPP

#include "VulkanWrapper/VulkanWrapperAPI.hpp"

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"

#include "ShaderReflection.hpp"
#include "VulkanObject.hpp"

#include <filesystem>
#include <functional>
#include <set>

#include <vulkan/vulkan.h>

namespace GVK {

class VULKANWRAPPER_API ShaderCompileException : public std::runtime_error {
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


VULKANWRAPPER_API
std::string ShaderKindToString (ShaderKind);


class VULKANWRAPPER_API ShaderModule : public VulkanObject {
public:
    static constexpr uint32_t ShaderKindCount = 6;

    enum class ReadMode {
        GLSLFilePath,
        SPVFilePath,
        GLSLString,
    };

    struct VULKANWRAPPER_API Reflection {
        ShaderKind shaderKind;

        std::vector<std::shared_ptr<SR::UBO>> ubos;
        std::vector<SR::Sampler>              samplers;
        std::vector<SR::Input>                inputs;
        std::vector<SR::Output>               outputs;
        std::vector<SR::SubpassInput>         subpassInputs;

        Reflection (ShaderKind shaderKind, const std::vector<uint32_t>& binary);


        class VULKANWRAPPER_API IDescriptorWriteInfoProvider {
        public:
            virtual ~IDescriptorWriteInfoProvider () = default;

            virtual std::vector<VkDescriptorImageInfo>  GetDescriptorImageInfos (const std::string& name, ShaderKind shaderKind, uint32_t layerIndex, uint32_t frameIndex) = 0;
            virtual std::vector<VkDescriptorBufferInfo> GetDescriptorBufferInfos (const std::string& name, ShaderKind shaderKind, uint32_t frameIndex) = 0;
        };

        struct VULKANWRAPPER_API DescriptorImageInfoTableEntry {
            std::string                                     name;
            ShaderKind                                      shaderKind;
            std::function<VkSampler ()>                     sampler;
            std::function<VkImageView (uint32_t, uint32_t)> imageView;
            VkImageLayout                                   imageLayout;
        };

        struct VULKANWRAPPER_API DescriptorBufferInfoTableEntry {
            std::string                        name;
            ShaderKind                         shaderKind;
            std::function<VkBuffer (uint32_t)> buffer;
            VkDeviceSize                       offset;
            VkDeviceSize                       range;
        };

        class VULKANWRAPPER_API DescriptorWriteInfoTable : public GVK::ShaderModule::Reflection::IDescriptorWriteInfoProvider {
        public:
            std::vector<DescriptorImageInfoTableEntry>  imageInfos;
            std::vector<DescriptorBufferInfoTableEntry> bufferInfos;

            virtual std::vector<VkDescriptorImageInfo> GetDescriptorImageInfos (const std::string& name, ShaderKind shaderKind, uint32_t layerIndex, uint32_t frameIndex) override;

            virtual std::vector<VkDescriptorBufferInfo> GetDescriptorBufferInfos (const std::string& name, ShaderKind shaderKind, uint32_t frameIndex) override;
        };

        size_t WriteDescriptors (VkDevice device, VkDescriptorSet dstSet, uint32_t frameIndex, ShaderKind shaderKind, IDescriptorWriteInfoProvider& infoProvider) const;

        std::vector<VkDescriptorSetLayoutBinding> GetLayout () const;

        std::vector<VkVertexInputAttributeDescription> GetVertexAttributes (const std::function<bool (const std::string&)>& instanceNameProvider) const;
        std::vector<VkVertexInputBindingDescription>   GetVertexBindings (const std::function<bool (const std::string&)>& instanceNameProvider) const;
    };

private:
    VkDevice                        device;
    GVK::MovablePtr<VkShaderModule> handle;
    ReadMode                        readMode;
    ShaderKind                      shaderKind;

    std::string           sourceCode;
    std::filesystem::path fileLocation;
    std::vector<uint32_t> binary;

    Reflection reflection;

private:
    // dont use this ctor, use factories instead
    ShaderModule (ShaderKind                   shaderKind,
                  ReadMode                     mode,
                  VkDevice                     device,
                  VkShaderModule               handle,
                  const std::filesystem::path& fileLocation,
                  const std::vector<uint32_t>& binary,
                  const std::string&           sourceCode);

    ShaderModule (ShaderModule&&) = default;
    ShaderModule& operator= (ShaderModule&&) = default;

public:
    static std::unique_ptr<ShaderModule> CreateFromGLSLString (VkDevice device, ShaderKind shaderKind, const std::string& shaderSource);
    static std::unique_ptr<ShaderModule> CreateFromGLSLFile (VkDevice device, const std::filesystem::path& fileLocation);
    static std::unique_ptr<ShaderModule> CreateFromSPVFile (VkDevice device, ShaderKind shaderKind, const std::filesystem::path& fileLocation);

    virtual ~ShaderModule () override;

    virtual void* GetHandleForName () const override { return handle; }

    virtual VkObjectType GetObjectTypeForName () const override { return VK_OBJECT_TYPE_SHADER_MODULE; }

    void Reload ();

    const std::vector<uint32_t>& GetBinary () const { return binary; }

    const std::filesystem::path& GetLocation () const { return fileLocation; }

    ShaderKind GetShaderKind () const { return shaderKind; }

    ReadMode GetReadMode () const { return readMode; }

    VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo () const;

    const Reflection& GetReflection () const { return reflection; }

    const std::string& GetSourceCode () const { return sourceCode; }
};

} // namespace GVK

#endif