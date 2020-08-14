#ifndef RENDERPASS_HPP
#define RENDERPASS_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

USING_PTR (RenderPass);
class GEARSVK_API RenderPass : public VulkanObject {
private:
    const VkDevice device;
    VkRenderPass   handle;

public:
    USING_CREATE (RenderPass);

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

        if (ERROR (vkCreateRenderPass (device, &renderPassInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create renderpass");
        }
    }

    ~RenderPass ()
    {
        vkDestroyRenderPass (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkRenderPass () const
    {
        return handle;
    }
};

#endif