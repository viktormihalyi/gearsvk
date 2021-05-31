#include "Framebuffer.hpp"

#include "Utils.hpp"
#include "Assert.hpp"

namespace GVK {


Framebuffer::Framebuffer (VkDevice device, VkRenderPass renderPass, const std::vector<std::reference_wrapper<ImageView2D>>& attachments, uint32_t width, uint32_t height)
    : Framebuffer (device, renderPass, Utils::ConvertToHandles<ImageView2D, VkImageView> (attachments), width, height)
{
}


Framebuffer::Framebuffer (VkDevice device, VkRenderPass renderPass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height)
    : device (device)
    , handle (VK_NULL_HANDLE)
    , width (width)
    , height (height)
{
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.flags                   = 0;
    framebufferInfo.renderPass              = renderPass;
    framebufferInfo.attachmentCount         = static_cast<uint32_t> (attachments.size ());
    framebufferInfo.pAttachments            = attachments.data ();
    framebufferInfo.width                   = width;
    framebufferInfo.height                  = height;
    framebufferInfo.layers                  = 1;

    VkResult result = vkCreateFramebuffer (device, &framebufferInfo, nullptr, &handle);
    if (GVK_ERROR (result != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create framebuffer");
    }
}


Framebuffer::~Framebuffer ()
{
    if (handle != nullptr) {
        vkDestroyFramebuffer (device, handle, nullptr);
        handle = nullptr;
    }
}


} // namespace GVK
