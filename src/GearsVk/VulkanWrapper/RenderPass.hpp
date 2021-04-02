#ifndef RENDERPASS_HPP
#define RENDERPASS_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "MovablePtr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

namespace GVK {

class GVK_RENDERER_API RenderPass : public VulkanObject {
private:
    VkDevice                      device;
    GVK::MovablePtr<VkRenderPass> handle;

public:
    RenderPass (VkDevice                                    device,
                const std::vector<VkAttachmentDescription>& attachments,
                const std::vector<VkSubpassDescription>&    subpasses,
                const std::vector<VkSubpassDependency>&     subpassDependencies)
        : device (device)
    {
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount        = static_cast<uint32_t> (attachments.size ());
        renderPassInfo.pAttachments           = attachments.data ();
        renderPassInfo.subpassCount           = static_cast<uint32_t> (subpasses.size ());
        renderPassInfo.pSubpasses             = subpasses.data ();
        renderPassInfo.dependencyCount        = static_cast<uint32_t> (subpassDependencies.size ());
        renderPassInfo.pDependencies          = subpassDependencies.data ();

        if (GVK_ERROR (vkCreateRenderPass (device, &renderPassInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create renderpass");
        }
    }

    virtual ~RenderPass () override
    {
        vkDestroyRenderPass (device, handle, nullptr);
        handle = nullptr;
    }

    operator VkRenderPass () const
    {
        return handle;
    }
};

} // namespace GVK

#endif