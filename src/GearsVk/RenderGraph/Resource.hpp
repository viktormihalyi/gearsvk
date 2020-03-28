#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "Shaderpipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "Connections.hpp"
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

class IImageResource {
public:
    USING_PTR_ABSTRACT (IImageResource);
    virtual ~IImageResource () {}
    virtual VkImageLayout GetFinalLayout () const = 0;
    virtual VkFormat      GetFormat () const      = 0;
    virtual uint32_t      GetLayerCount () const  = 0;
};

class Resource : public Noncopyable, public IImageResource {
public:
    USING_PTR_ABSTRACT (Resource);

    virtual ~Resource () {}

    virtual void WriteToDescriptorSet (uint32_t imageIndex, const DescriptorSet& descriptorSet, uint32_t binding) const = 0;
    virtual void BindRead (uint32_t imageIndex, VkCommandBuffer commandBuffer)                                          = 0;
    virtual void BindWrite (uint32_t imageIndex, VkCommandBuffer commandBuffer)                                         = 0;
};


struct SingleImageResource final : public SingleResource {
    static const VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB;

    const AllocatedImage         image;
    std::vector<ImageView::U>    imageViews;
    const Sampler::U             sampler;
    std::optional<VkImageLayout> layoutRead;
    std::optional<VkImageLayout> layoutWrite;

    // write always happens before read
    // NO  read, NO  write: general
    // YES read, NO  write: general -> read
    // NO  read, YES write: general -> write
    // YES read, YES write: general -> write -> read

    USING_PTR (SingleImageResource);

    SingleImageResource (const GraphSettings& graphSettings, uint32_t arrayLayers);

    virtual ~SingleImageResource () {}

    virtual void WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const override;
    virtual void BindRead (VkCommandBuffer commandBuffer) override;
    virtual void BindWrite (VkCommandBuffer commandBuffer) override;
};


class ImageResource : public Resource {
public:
    uint32_t                            arrayLayers;
    std::vector<SingleImageResource::U> images;

public:
    USING_PTR (ImageResource);

    ImageResource (const GraphSettings& graphSettings, uint32_t arrayLayers = 1)
        : arrayLayers (arrayLayers)
    {
        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            images.push_back (SingleImageResource::Create (graphSettings, arrayLayers));
        }
    }

    virtual ~ImageResource () {}

    virtual void WriteToDescriptorSet (uint32_t imageIndex, const DescriptorSet& descriptorSet, uint32_t binding) const override { images[imageIndex]->WriteToDescriptorSet (descriptorSet, binding); }
    virtual void BindRead (uint32_t imageIndex, VkCommandBuffer commandBuffer) override { images[imageIndex]->BindRead (commandBuffer); }
    virtual void BindWrite (uint32_t imageIndex, VkCommandBuffer commandBuffer) override { images[imageIndex]->BindWrite (commandBuffer); }

    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; }
    virtual VkFormat      GetFormat () const override { return SingleImageResource::Format; }
    virtual uint32_t      GetLayerCount () const override { return arrayLayers; }
};


class SwapchainImageResource : public Resource {
public:
    VkFormat                  swapchainSurfaceFormat;
    std::vector<ImageView::U> imageViews;

public:
    USING_PTR (SwapchainImageResource);
    SwapchainImageResource (const GraphSettings& graphSettings, Swapchain& swapchain)
        : swapchainSurfaceFormat (swapchain.GetImageFormat ())
    {
        uint32_t imageCount;
        vkGetSwapchainImagesKHR (graphSettings.device, swapchain, &imageCount, nullptr);
        std::vector<VkImage> swapChainImages (imageCount);
        vkGetSwapchainImagesKHR (graphSettings.device, swapchain, &imageCount, swapChainImages.data ());

        for (size_t i = 0; i < swapChainImages.size (); ++i) {
            imageViews.push_back (ImageView::Create (graphSettings.device, swapChainImages[i], swapchain.GetImageFormat ()));
        }
    }

    virtual ~SwapchainImageResource () {}

    virtual void WriteToDescriptorSet (uint32_t imageIndex, const DescriptorSet& descriptorSet, uint32_t binding) const override {}
    virtual void BindRead (uint32_t imageIndex, VkCommandBuffer commandBuffer) override {}
    virtual void BindWrite (uint32_t imageIndex, VkCommandBuffer commandBuffer) override {}

    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; }
    virtual VkFormat      GetFormat () const override { return swapchainSurfaceFormat; }
    virtual uint32_t      GetLayerCount () const override { return 1; }
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