#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include "Assert.hpp"
#include "ImageView.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>

class Framebuffer : public Noncopyable {
private:
    const VkDevice device;
    VkFramebuffer  handle;

    const uint32_t width;
    const uint32_t height;

public:
    USING_PTR (Framebuffer);

    Framebuffer (VkDevice device, VkRenderPass renderPass, const std::vector<std::reference_wrapper<ImageView>>& attachments, uint32_t width, uint32_t height)
        : Framebuffer (device, renderPass, Utils::ConvertToHandles<ImageView, VkImageView> (attachments), width, height)
    {
    }

    Framebuffer (VkDevice device, VkRenderPass renderPass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height)
        : device (device)
        , handle (VK_NULL_HANDLE)
        , width (width)
        , height (height)
    {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.flags                   = 0;
        framebufferInfo.renderPass              = renderPass;
        framebufferInfo.attachmentCount         = attachments.size ();
        framebufferInfo.pAttachments            = attachments.data ();
        framebufferInfo.width                   = width;
        framebufferInfo.height                  = height;
        framebufferInfo.layers                  = 1;

        VkResult result = vkCreateFramebuffer (device, &framebufferInfo, nullptr, &handle);
        if (ERROR (result != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create framebuffer");
        }
    }

    ~Framebuffer ()
    {
        vkDestroyFramebuffer (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    const size_t GetWidth () const
    {
        return width;
    }

    const size_t GetHeight () const
    {
        return height;
    }

    operator VkFramebuffer () const
    {
        return handle;
    }
};

#endif