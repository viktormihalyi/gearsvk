#ifndef RENDERGRAPH_SHADERREFLECTIONTODESCRIPTOR_HPP
#define RENDERGRAPH_SHADERREFLECTIONTODESCRIPTOR_HPP

#include "RenderGraphAPI.hpp"

#include "VulkanWrapper/ShaderModule.hpp"

#include <vulkan/vulkan.h>

#include <functional>
#include <string>
#include <vector>
#include <cstdint>


namespace RG {
namespace FromShaderReflection {

class GVK_RENDERER_API IDescriptorWriteInfoProvider {
public:
    virtual ~IDescriptorWriteInfoProvider ();

    virtual std::vector<VkDescriptorImageInfo>  GetDescriptorImageInfos (const std::string& name, GVK::ShaderKind shaderKind, uint32_t layerIndex, uint32_t frameIndex) = 0;
    virtual std::vector<VkDescriptorBufferInfo> GetDescriptorBufferInfos (const std::string& name, GVK::ShaderKind shaderKind, uint32_t frameIndex)                     = 0;
};


class GVK_RENDERER_API DescriptorWriteInfoTable : public IDescriptorWriteInfoProvider {
public:
    struct ImageEntry {
        std::string                                     name;
        GVK::ShaderKind                                 shaderKind;
        std::function<VkSampler ()>                     sampler;
        std::function<VkImageView (uint32_t, uint32_t)> imageView;
        VkImageLayout                                   imageLayout;
    };

    struct BufferEntry {
        std::string                        name;
        GVK::ShaderKind                    shaderKind;
        std::function<VkBuffer (uint32_t)> buffer;
        VkDeviceSize                       offset;
        VkDeviceSize                       range;
    };

    std::vector<ImageEntry> imageInfos;
    std::vector<BufferEntry> bufferInfos;

    virtual ~DescriptorWriteInfoTable () override = default;

    virtual std::vector<VkDescriptorImageInfo> GetDescriptorImageInfos (const std::string& name, GVK::ShaderKind shaderKind, uint32_t layerIndex, uint32_t frameIndex) override;

    virtual std::vector<VkDescriptorBufferInfo> GetDescriptorBufferInfos (const std::string& name, GVK::ShaderKind shaderKind, uint32_t frameIndex) override;
};


class GVK_RENDERER_API IUpdateDescriptorSets {
public:
    virtual ~IUpdateDescriptorSets ();

    virtual void UpdateDescriptorSets (const std::vector<VkWriteDescriptorSet>& writes) = 0;
};


GVK_RENDERER_API
void WriteDescriptors (const GVK::ShaderModule::Reflection& reflection,
                       VkDescriptorSet                      dstSet,
                       uint32_t                             frameIndex,
                       GVK::ShaderKind                      shaderKind,
                       IDescriptorWriteInfoProvider&        infoProvider,
                       IUpdateDescriptorSets&               updateInterface);


GVK_RENDERER_API
std::vector<VkDescriptorSetLayoutBinding> GetLayout (const GVK::ShaderModule::Reflection& reflection, GVK::ShaderKind shaderKind);

} // namespace FromShaderReflection
} // namespace RG

#endif
