#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "Shaderpipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "GraphSettings.hpp"

namespace RenderGraph {

class SingleResource : public Noncopyable {
public:
    USING_PTR_ABSTRACT (SingleResource);

    virtual ~SingleResource () {}
    virtual void WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const = 0;
    virtual void BindRead (VkCommandBuffer commandBuffer)                                          = 0;
    virtual void BindWrite (VkCommandBuffer commandBuffer)                                         = 0;
};


struct SingleImageResource final : public SingleResource {
    static const VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB;

    AllocatedImage image;
    ImageView::U   imageView;
    Sampler::U     sampler;


    // write always happens before read
    // NO  read, NO  write: general
    // YES read, NO  write: general -> read
    // NO  read, YES write: general -> write
    // YES read, YES write: general -> write -> read

    std::optional<VkImageLayout> layoutRead;
    std::optional<VkImageLayout> layoutWrite;

    USING_PTR (SingleImageResource);

    SingleImageResource (VkDevice device, Image::U&& image);

    SingleImageResource (const GraphSettings& graphSettings, const Device& device, VkQueue queue, VkCommandPool commandPool, std::optional<VkImageLayout> layoutRead, std::optional<VkImageLayout> layoutWrite);

    virtual ~SingleImageResource () {}

    virtual void WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const override;

    virtual void BindRead (VkCommandBuffer commandBuffer) override;
    virtual void BindWrite (VkCommandBuffer commandBuffer) override;
};


struct OutputConnectionInfo {
    VkFormat      format;
    VkImageLayout finalLayout;
};


class Resource : public Noncopyable {
public:
    USING_PTR_ABSTRACT (Resource);

    virtual ~Resource () {}
    virtual void                 WriteToDescriptorSet (uint32_t imageIndex, const DescriptorSet& descriptorSet, uint32_t binding) const = 0;
    virtual void                 BindRead (uint32_t imageIndex, VkCommandBuffer commandBuffer)                                          = 0;
    virtual void                 BindWrite (uint32_t imageIndex, VkCommandBuffer commandBuffer)                                         = 0;
    virtual OutputConnectionInfo GetOutputConnectionInfo () const                                                                       = 0;
};


class ImageResource : public Resource {
public:
    std::vector<SingleImageResource::U> images;

public:
    USING_PTR (ImageResource);

    ImageResource (const GraphSettings& graphSettings, const Device& device, VkQueue queue, VkCommandPool commandPool, std::optional<VkImageLayout> layoutRead, std::optional<VkImageLayout> layoutWrite)
    {
        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            images.push_back (SingleImageResource::Create (graphSettings, device, queue, commandPool, layoutRead, layoutWrite));
        }
    }

    virtual OutputConnectionInfo GetOutputConnectionInfo () const override
    {
        return {SingleImageResource::Format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    }

    virtual ~ImageResource () {}
    virtual void WriteToDescriptorSet (uint32_t imageIndex, const DescriptorSet& descriptorSet, uint32_t binding) const override { images[imageIndex]->WriteToDescriptorSet (descriptorSet, binding); }
    virtual void BindRead (uint32_t imageIndex, VkCommandBuffer commandBuffer) override { images[imageIndex]->BindRead (commandBuffer); }
    virtual void BindWrite (uint32_t imageIndex, VkCommandBuffer commandBuffer) override { images[imageIndex]->BindWrite (commandBuffer); }
};

class SwapchainImageResource : public Resource {
public:
    VkFormat                  swapchainSurfaceFormat;
    std::vector<ImageView::U> imageViews;

public:
    USING_PTR (SwapchainImageResource);
    SwapchainImageResource (VkDevice device, Swapchain& swapchain)
        : swapchainSurfaceFormat (swapchain.GetImageFormat ())
    {
        uint32_t imageCount;
        vkGetSwapchainImagesKHR (device, swapchain, &imageCount, nullptr);
        std::vector<VkImage> swapChainImages (imageCount);
        vkGetSwapchainImagesKHR (device, swapchain, &imageCount, swapChainImages.data ());

        for (size_t i = 0; i < swapChainImages.size (); ++i) {
            imageViews.push_back (ImageView::Create (device, swapChainImages[i], swapchain.GetImageFormat ()));
        }
    }

    virtual ~SwapchainImageResource ()
    {
    }

    virtual OutputConnectionInfo GetOutputConnectionInfo () const override
    {
        return {swapchainSurfaceFormat, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
    }

    virtual void WriteToDescriptorSet (uint32_t imageIndex, const DescriptorSet& descriptorSet, uint32_t binding) const override {}
    virtual void BindRead (uint32_t imageIndex, VkCommandBuffer commandBuffer) override {}
    virtual void BindWrite (uint32_t imageIndex, VkCommandBuffer commandBuffer) override {}
};

struct ResourceVisitor final {
private:
    template<typename T>
    using VisitorCallback = const std::function<void (T&)>&;

public:
    static void Visit (Resource&,
                       VisitorCallback<ImageResource>,
                       VisitorCallback<SwapchainImageResource>);
};

} // namespace RenderGraph

#endif