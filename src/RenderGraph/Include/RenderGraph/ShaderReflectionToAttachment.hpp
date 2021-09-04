#ifndef RENDERGRAPH_SHADERREFLECTIONTOATTACHMENT_HPP
#define RENDERGRAPH_SHADERREFLECTIONTOATTACHMENT_HPP

#include "RenderGraphAPI.hpp"

#include "VulkanWrapper/ShaderModule.hpp"

#include <vulkan/vulkan.h>

#include <functional>
#include <optional>


namespace RG {
namespace FromShaderReflection {

class GVK_RENDERER_API IAttachmentProvider {
public:
    virtual ~IAttachmentProvider ();

    struct AttachmentData {
        std::function<VkFormat ()>                      format;
        VkAttachmentLoadOp                              loadOp;
        std::function<VkImageView (uint32_t, uint32_t)> imageView;
        VkImageLayout                                   initialLayout;
        VkImageLayout                                   finalLayout;
    };

    virtual std::optional<AttachmentData> GetAttachmentData (const std::string& name, GVK::ShaderKind shaderKind) = 0;
};

class GVK_RENDERER_API AttachmentDataTable : public IAttachmentProvider {
public:
    struct AttachmentDataEntry {
        std::string     name;
        GVK::ShaderKind shaderKind;
        AttachmentData  data;
    };

    std::vector<AttachmentDataEntry> table;

    virtual ~AttachmentDataTable () override = default;

    virtual std::optional<AttachmentData> GetAttachmentData (const std::string& name, GVK::ShaderKind shaderKind) override;
};


GVK_RENDERER_API
std::vector<VkImageView> GetImageViews (const GVK::ShaderModule::Reflection& reflection, GVK::ShaderKind shaderKind, uint32_t resourceIndex, IAttachmentProvider& attachmentProvider);


GVK_RENDERER_API
std::vector<VkAttachmentReference>   GetAttachmentReferences (const GVK::ShaderModule::Reflection& reflection, GVK::ShaderKind shaderKind, IAttachmentProvider& attachmentProvider);


GVK_RENDERER_API
std::vector<VkAttachmentDescription> GetAttachmentDescriptions (const GVK::ShaderModule::Reflection& reflection, GVK::ShaderKind shaderKind, IAttachmentProvider& attachmentProvider);


} // namespace FromShaderReflection
} // namespace RG

#endif
