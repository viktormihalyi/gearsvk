#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include "ImageView.hpp"
#include "Utils/MovablePtr.hpp"
#include "VulkanObject.hpp"
#include "VulkanWrapper/ImageView.hpp"

#include <vulkan/vulkan.h>

namespace GVK {

class VULKANWRAPPER_API Framebuffer final : public VulkanObject {
private:
    VkDevice                       device;
    GVK::MovablePtr<VkFramebuffer> handle;
    std::vector<ImageView2D>       attachments;

    uint32_t width;
    uint32_t height;

public:
    Framebuffer (VkDevice device, VkRenderPass renderPass, const std::vector<std::reference_wrapper<ImageView2D>>& attachments, uint32_t width, uint32_t height);
    Framebuffer (VkDevice device, VkRenderPass renderPass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height);
    Framebuffer (VkDevice device, VkRenderPass renderPass, std::vector<ImageView2D>&& attachments, uint32_t width, uint32_t height);

    Framebuffer (Framebuffer&&) = default;
    Framebuffer& operator= (Framebuffer&&) = default;

    virtual ~Framebuffer () override;

    const size_t GetWidth () const { return width; }
    const size_t GetHeight () const { return height; }

    operator VkFramebuffer () const { return handle; }
};

} // namespace GVK

#endif