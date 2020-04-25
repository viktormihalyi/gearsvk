#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "Event.hpp"
#include "ShaderPipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "Connections.hpp"
#include "GraphSettings.hpp"
#include "UniformBlock.hpp"

namespace RG {


class SingleResource : public Noncopyable {
public:
    USING_PTR_ABSTRACT (SingleResource);

    virtual ~SingleResource () {}

    virtual VkDescriptorType GetDescriptorType () const                                                        = 0;
    virtual void             WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const = 0;
    virtual void             BindRead (VkCommandBuffer commandBuffer)                                          = 0;
    virtual void             BindWrite (VkCommandBuffer commandBuffer)                                         = 0;
};


class IImageResource {
public:
    USING_PTR_ABSTRACT (IImageResource);
    virtual ~IImageResource () {}
    virtual VkImageLayout GetFinalLayout () const     = 0;
    virtual VkFormat      GetFormat () const          = 0;
    virtual uint32_t      GetDescriptorCount () const = 0;
};

class Resource : public Noncopyable, public IImageResource {
public:
    USING_PTR_ABSTRACT (Resource);

    virtual ~Resource () = default;

    virtual VkDescriptorType GetDescriptorType () const                                                                             = 0;
    virtual void             WriteToDescriptorSet (uint32_t imageIndex, const DescriptorSet& descriptorSet, uint32_t binding) const = 0;
    virtual void             BindRead (uint32_t imageIndex, VkCommandBuffer commandBuffer)                                          = 0;
    virtual void             BindWrite (uint32_t imageIndex, VkCommandBuffer commandBuffer)                                         = 0;
    virtual void             Compile (const GraphSettings&)                                                                         = 0;
};


class OneTimeCompileResource : public Resource {
private:
    bool compiled;

public:
    USING_PTR_ABSTRACT (OneTimeCompileResource);

    OneTimeCompileResource ()
        : compiled (false)
    {
    }

    virtual ~OneTimeCompileResource () = default;

    void Compile (const GraphSettings& settings) override
    {
        if (!compiled) {
            compiled = true;
            CompileOnce (settings);
        }
    }

    virtual void CompileOnce (const GraphSettings&) = 0;
};


struct SingleImageResource final : public SingleResource {
    static const VkFormat FormatRGBA;
    static const VkFormat FormatRGB;

    const AllocatedImage         image;
    std::vector<ImageView2D::U>  imageViews;
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

    virtual VkDescriptorType GetDescriptorType () const override { return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; }
    virtual void             WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const override;
    virtual void             BindRead (VkCommandBuffer commandBuffer) override;
    virtual void             BindWrite (VkCommandBuffer commandBuffer) override;
};


class WritableImageResource : public Resource {
public:
    uint32_t                            arrayLayers;
    std::vector<SingleImageResource::U> images;

public:
    USING_PTR (WritableImageResource);

    WritableImageResource (uint32_t arrayLayers = 1)
        : arrayLayers (arrayLayers)
    {
    }

    virtual ~WritableImageResource () {}

    virtual VkDescriptorType GetDescriptorType () const override { return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; }
    virtual void             WriteToDescriptorSet (uint32_t imageIndex, const DescriptorSet& descriptorSet, uint32_t binding) const override { images[imageIndex]->WriteToDescriptorSet (descriptorSet, binding); }
    virtual void             BindRead (uint32_t imageIndex, VkCommandBuffer commandBuffer) override { images[imageIndex]->BindRead (commandBuffer); }
    virtual void             BindWrite (uint32_t imageIndex, VkCommandBuffer commandBuffer) override { images[imageIndex]->BindWrite (commandBuffer); }

    virtual void Compile (const GraphSettings& graphSettings)
    {
        images.clear ();
        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            images.push_back (SingleImageResource::Create (graphSettings, arrayLayers));
        }
    }

    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; }
    virtual VkFormat      GetFormat () const override { return SingleImageResource::FormatRGBA; }
    virtual uint32_t      GetDescriptorCount () const override { return arrayLayers; }
};


class ReadOnlyImageResource : public OneTimeCompileResource {
public:
    ImageTransferableBase::U image;
    ImageViewBase::U         imageView;
    Sampler::U               sampler;

    const VkFormat format;
    const uint32_t width;
    const uint32_t height;
    const uint32_t depth;

public:
    USING_PTR (ReadOnlyImageResource);

    ReadOnlyImageResource (VkFormat format, uint32_t width, uint32_t height = 1, uint32_t depth = 1)
        : format (format)
        , width (width)
        , height (height)
        , depth (depth)
    {
        ASSERT_THROW (width > 0);
        ASSERT_THROW (height > 0);
        ASSERT_THROW (depth > 0);
    }

    virtual ~ReadOnlyImageResource () {}

    virtual VkDescriptorType GetDescriptorType () const override { return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; }

    virtual void WriteToDescriptorSet (uint32_t imageIndex, const DescriptorSet& descriptorSet, uint32_t binding) const override
    {
        ASSERT (image != nullptr);

        descriptorSet.WriteOneImageInfo (
            binding,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            {*sampler, *imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    }

    virtual void BindRead (uint32_t, VkCommandBuffer) override {}
    virtual void BindWrite (uint32_t, VkCommandBuffer) override {}

    virtual void CompileOnce (const GraphSettings& settings)
    {
        sampler = Sampler::Create (settings.GetDevice ());

        if (height == 1 && depth == 1) {
            image     = Image1DTransferable::Create (settings.GetDevice (), settings.queue, settings.commandPool, format, width, VK_IMAGE_USAGE_SAMPLED_BIT);
            imageView = ImageView1D::Create (settings.GetDevice (), *image->imageGPU->image);
        } else if (depth == 1) {
            image     = Image2DTransferable::Create (settings.GetDevice (), settings.queue, settings.commandPool, format, width, height, VK_IMAGE_USAGE_SAMPLED_BIT);
            imageView = ImageView2D::Create (settings.GetDevice (), *image->imageGPU->image);
        } else {
            image     = Image3DTransferable::Create (settings.GetDevice (), settings.queue, settings.commandPool, format, width, height, depth, VK_IMAGE_USAGE_SAMPLED_BIT);
            imageView = ImageView3D::Create (settings.GetDevice (), *image->imageGPU->image);
        }

        SingleTimeCommand s (settings.GetDevice (), settings.commandPool, settings.queue);
        image->imageGPU->image->CmdPipelineBarrier (s, ImageBase::INITIAL_LAYOUT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; }
    virtual VkFormat      GetFormat () const override { return format; }
    virtual uint32_t      GetDescriptorCount () const override { return 1; }

    void CopyTransitionTransfer (const std::vector<uint8_t>& pixelData)
    {
        image->CopyTransitionTransfer (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, pixelData.data (), pixelData.size (), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
};


class SwapchainImageResource : public Resource {
public:
    VkFormat                    swapchainSurfaceFormat;
    std::vector<ImageView2D::U> imageViews;
    Swapchain&                  swapchain;

public:
    USING_PTR (SwapchainImageResource);
    SwapchainImageResource (Swapchain& swapchain)
        : swapchain (swapchain)
    {
    }

    virtual ~SwapchainImageResource () {}

    virtual VkDescriptorType GetDescriptorType () const override { return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; }
    virtual void             WriteToDescriptorSet (uint32_t, const DescriptorSet&, uint32_t) const override {}
    virtual void             BindRead (uint32_t, VkCommandBuffer) override {}
    virtual void             BindWrite (uint32_t, VkCommandBuffer) override {}

    virtual void Compile (const GraphSettings& graphSettings) override
    {
        swapchainSurfaceFormat = swapchain.GetImageFormat ();

        std::vector<VkImage> swapChainImages = swapchain.GetImages ();

        imageViews.clear ();
        for (size_t i = 0; i < swapChainImages.size (); ++i) {
            imageViews.push_back (ImageView2D::Create (graphSettings.GetDevice (), swapChainImages[i], swapchain.GetImageFormat ()));
        }
    }

    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; }
    virtual VkFormat      GetFormat () const override { return swapchainSurfaceFormat; }
    virtual uint32_t      GetDescriptorCount () const override { return 1; }
};


class UniformBlockResource : public Resource {
public:
    const uint32_t                  size;
    std::vector<AllocatedBuffer::U> buffers;
    std::vector<MemoryMapping::U>   mappings;

public:
    USING_PTR (UniformBlockResource);

    UniformBlockResource (uint32_t size)
        : size (size)
    {
    }

    UniformBlockResource (const ShaderStruct& structType)
        : size (structType.GetFullSize ())
    {
    }

    virtual ~UniformBlockResource () {}

    virtual VkDescriptorType GetDescriptorType () const override { return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; }
    virtual void             WriteToDescriptorSet (uint32_t imageIndex, const DescriptorSet& descriptorSet, uint32_t binding) const override
    {
        VkDescriptorBufferInfo desc;
        desc.buffer = *buffers[imageIndex]->buffer;
        desc.offset = 0;
        desc.range  = size;

        descriptorSet.WriteOneBufferInfo (binding, GetDescriptorType (), desc);
    }


    virtual void BindRead (uint32_t, VkCommandBuffer) override {}
    virtual void BindWrite (uint32_t, VkCommandBuffer) override {}

    virtual void Compile (const GraphSettings& graphSettings) override
    {
        for (uint32_t i = 0; i < graphSettings.framesInFlight; ++i) {
            buffers.push_back (AllocatedBuffer::Create (graphSettings.GetDevice (), UniformBuffer::Create (graphSettings.GetDevice (), size), DeviceMemory::CPU));
            mappings.push_back (MemoryMapping::Create (graphSettings.GetDevice (), *buffers[buffers.size () - 1]->memory, 0, size));
        }
    }

    virtual VkImageLayout GetFinalLayout () const override { throw std::runtime_error ("not an img"); }
    virtual VkFormat      GetFormat () const override { throw std::runtime_error ("not an img"); }
    virtual uint32_t      GetDescriptorCount () const override { return 1; }

    MemoryMapping& GetMapping (uint32_t frameIndex) { return *mappings[frameIndex]; }

    void Set (uint32_t frameIndex, UniformBlock& uniformBlock)
    {
        ASSERT (uniformBlock.GetSize () == size);
        GetMapping (frameIndex).Copy (uniformBlock.GetData (), uniformBlock.GetSize (), 0);
    }
};


class ResourceVisitor final {
public:
    Event<WritableImageResource&>  onWritableImage;
    Event<ReadOnlyImageResource&>  onReadOnlyImage;
    Event<SwapchainImageResource&> onSwapchainImage;
    Event<UniformBlockResource&>   onUniformImage;

    void Visit (Resource& res);

    void Visit (Resource::Ref& res) { Visit (res); }

    template<typename Resources>
    void VisitAll (const Resources& resources)
    {
        for (auto& r : resources) {
            Visit (r);
        }
    }
};

} // namespace RG

#endif