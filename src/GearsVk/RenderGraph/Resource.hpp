#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "GearsVkAPI.hpp"

#include "Event.hpp"
#include "ShaderPipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "Connections.hpp"
#include "GraphSettings.hpp"
#include "InputBindable.hpp"
#include "Node.hpp"
#include "Ptr.hpp"


namespace RG {

class GraphSettings;


USING_PTR (Resource);
class GEARSVK_API Resource : public Node {
public:
    virtual ~Resource () = default;

    virtual void Compile (const GraphSettings&) = 0;
};


USING_PTR (InputBufferBindableResource);
class GEARSVK_API InputBufferBindableResource : public Resource, public InputBufferBindable {
public:
    virtual ~InputBufferBindableResource () = default;
};


USING_PTR (ImageResource);
class GEARSVK_API ImageResource : public Resource {
public:
    virtual ~ImageResource () = default;

    virtual void                    BindRead (uint32_t imageIndex, CommandBuffer& commandBuffer)  = 0;
    virtual void                    BindWrite (uint32_t imageIndex, CommandBuffer& commandBuffer) = 0;
    virtual VkImageLayout           GetFinalLayout () const                                       = 0;
    virtual VkFormat                GetFormat () const                                            = 0;
    virtual uint32_t                GetDescriptorCount () const                                   = 0;
    virtual std::vector<ImageBase*> GetImages () const                                            = 0;
};


USING_PTR (OneTimeCompileResource);
class GEARSVK_API OneTimeCompileResource : public ImageResource {
private:
    bool compiled;

protected:
    OneTimeCompileResource ()
        : compiled (false)
    {
    }

public:
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


USING_PTR (WritableImageResource);
class GEARSVK_API WritableImageResource : public ImageResource, public InputImageBindable {
    USING_CREATE (WritableImageResource);

private:
    SamplerU sampler;

    USING_PTR (SingleImageResource);
    struct SingleImageResource final {
        USING_CREATE (SingleImageResource);

        static const VkFormat FormatRGBA;
        static const VkFormat FormatRGB;

        const ImageBaseU             image;
        std::vector<ImageView2DU>    imageViews;
        std::optional<VkImageLayout> layoutRead;
        std::optional<VkImageLayout> layoutWrite;

        // write always happens before read
        // NO  read, NO  write: general
        // YES read, NO  write: general -> read
        // NO  read, YES write: general -> write
        // YES read, YES write: general -> write -> read


        SingleImageResource (const GraphSettings& graphSettings, uint32_t width, uint32_t height, uint32_t arrayLayers)
            : image (Image2D::Create (graphSettings.GetDevice ().GetAllocator (), ImageBase::MemoryLocation::GPU, width, height, FormatRGBA, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, arrayLayers))
        {
            for (uint32_t layerIndex = 0; layerIndex < arrayLayers; ++layerIndex) {
                imageViews.push_back (ImageView2D::Create (graphSettings.GetDevice (), *image, layerIndex));
            }
        }
    };

public:
    uint32_t                          width;
    uint32_t                          height;
    uint32_t                          arrayLayers;
    std::vector<SingleImageResourceU> images;

public:
    WritableImageResource (uint32_t width, uint32_t height, uint32_t arrayLayers = 1)
        : width (width)
        , height (height)
        , arrayLayers (arrayLayers)
    {
    }

    virtual ~WritableImageResource () {}

    virtual void BindRead (uint32_t imageIndex, CommandBuffer& commandBuffer) override
    {
        SingleImageResource& im = *images[imageIndex];

        im.layoutRead                = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkImageLayout previousLayout = (im.layoutWrite.has_value ()) ? *im.layoutWrite : ImageBase::INITIAL_LAYOUT;
        im.image->CmdPipelineBarrier (commandBuffer, previousLayout, *im.layoutRead);
    }

    virtual void BindWrite (uint32_t imageIndex, CommandBuffer& commandBuffer) override
    {
        SingleImageResource& im = *images[imageIndex];

        im.layoutWrite = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        im.image->CmdPipelineBarrier (commandBuffer, ImageBase::INITIAL_LAYOUT, *im.layoutWrite);
    }

    virtual void Compile (const GraphSettings& graphSettings) override
    {
        sampler = Sampler::Create (graphSettings.GetDevice ());

        images.clear ();
        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            images.push_back (SingleImageResource::Create (graphSettings, width, height, arrayLayers));
        }
    }

    // overriding ImageResource
    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; }
    virtual VkFormat      GetFormat () const override { return SingleImageResource::FormatRGBA; }
    virtual uint32_t      GetDescriptorCount () const override { return arrayLayers; }

    virtual std::vector<ImageBase*> GetImages () const override
    {
        std::vector<ImageBase*> result;

        for (auto& img : images) {
            result.push_back (img->image.get ());
        }

        return result;
    }

    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t frameIndex, uint32_t layerIndex) override { return *images[frameIndex]->imageViews[layerIndex]; }
    virtual VkSampler   GetSampler () override { return *sampler; }
};


USING_PTR (GPUBufferResource);
class GEARSVK_API GPUBufferResource : public InputBufferBindableResource {
    USING_CREATE (GPUBufferResource);

private:
    BufferTransferableU buffer;
    const uint32_t      size;

public:
    GPUBufferResource (const uint32_t size)
        : size (size)
    {
    }

    virtual ~GPUBufferResource () = default;

    virtual void Compile (const GraphSettings& settings) override
    {
        buffer = BufferTransferable::Create (settings.GetDevice (), size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    }

    // overriding InputImageBindable
    virtual VkBuffer GetBufferForFrame (uint32_t) override
    {
        return buffer->bufferGPU;
    }

    virtual uint32_t GetBufferSize () override
    {
        return size;
    }

    void CopyAndTransfer (const void* data, size_t size) const
    {
        buffer->CopyAndTransfer (data, size);
    }
};


USING_PTR (ReadOnlyImageResource);
class GEARSVK_API ReadOnlyImageResource : public OneTimeCompileResource, public InputImageBindable {
public:
    ImageTransferableBaseU image;
    ImageViewBaseU         imageView;
    SamplerU               sampler;

    const VkFormat format;
    const uint32_t width;
    const uint32_t height;
    const uint32_t depth;
    const uint32_t layerCount;

public:
    USING_CREATE (ReadOnlyImageResource);

    ReadOnlyImageResource (VkFormat format, uint32_t width, uint32_t height = 1, uint32_t depth = 1, uint32_t layerCount = 1)
        : format (format)
        , width (width)
        , height (height)
        , depth (depth)
        , layerCount (layerCount)
    {
        GVK_ASSERT_THROW (width > 0);
        GVK_ASSERT_THROW (height > 0);
        GVK_ASSERT_THROW (depth > 0);
        GVK_ASSERT_THROW (layerCount > 0);
    }

    virtual ~ReadOnlyImageResource () {}

    // overriding OneTimeCompileResource
    virtual void CompileOnce (const GraphSettings& settings) override
    {
        sampler = Sampler::Create (settings.GetDevice ());

        if (height == 1 && depth == 1) {
            image     = Image1DTransferable::Create (settings.GetDevice (), format, width, VK_IMAGE_USAGE_SAMPLED_BIT);
            imageView = ImageView1D::Create (settings.GetDevice (), *image->imageGPU);
        } else if (depth == 1) {
            if (layerCount == 1) {
                image     = Image2DTransferable::Create (settings.GetDevice (), format, width, height, VK_IMAGE_USAGE_SAMPLED_BIT);
                imageView = ImageView2D::Create (settings.GetDevice (), *image->imageGPU, 0);
            } else {
                image     = Image2DTransferable::Create (settings.GetDevice (), format, width, height, VK_IMAGE_USAGE_SAMPLED_BIT, layerCount);
                imageView = ImageView2DArray::Create (settings.GetDevice (), *image->imageGPU, 0, layerCount);
            }
        } else {
            image     = Image3DTransferable::Create (settings.GetDevice (), format, width, height, depth, VK_IMAGE_USAGE_SAMPLED_BIT);
            imageView = ImageView3D::Create (settings.GetDevice (), *image->imageGPU);
        }

        SingleTimeCommand s (settings.GetDevice ());
        image->imageGPU->CmdPipelineBarrier (s.Record (), ImageBase::INITIAL_LAYOUT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    // overriding ImageResource
    virtual void          BindRead (uint32_t, CommandBuffer&) override {}
    virtual void          BindWrite (uint32_t, CommandBuffer&) override {}
    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; }
    virtual VkFormat      GetFormat () const override { return format; }
    virtual uint32_t      GetDescriptorCount () const override { return 1; }

    virtual std::vector<ImageBase*> GetImages () const override
    {
        return { image->imageGPU.get () };
    }

    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t, uint32_t) override { return *imageView; }
    virtual VkSampler   GetSampler () override { return *sampler; }

    template<typename T>
    void CopyTransitionTransfer (const std::vector<T>& pixelData)
    {
        CopyLayer (pixelData, 0);
    }

    template<typename T>
    void CopyLayer (const std::vector<T>& pixelData, uint32_t layerIndex)
    {
        image->CopyLayer (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, pixelData.data (), pixelData.size () * sizeof (T), layerIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
};


USING_PTR (SwapchainImageResource);
class GEARSVK_API SwapchainImageResource : public ImageResource, public InputImageBindable {
    USING_CREATE (SwapchainImageResource);

public:
    std::vector<ImageView2DU>    imageViews;
    SwapchainProvider&           swapchainProv;
    std::vector<InheritedImageU> inheritedImages;

public:
    SwapchainImageResource (SwapchainProvider& swapchainProv)
        : swapchainProv (swapchainProv)
    {
    }

    virtual ~SwapchainImageResource () = default;

    // overriding Resource
    virtual void Compile (const GraphSettings& graphSettings) override
    {
        const std::vector<VkImage> swapChainImages = swapchainProv.GetSwapchain ().GetImages ();

        imageViews.clear ();
        for (size_t i = 0; i < swapChainImages.size (); ++i) {
            imageViews.push_back (ImageView2D::Create (graphSettings.GetDevice (), swapChainImages[i], swapchainProv.GetSwapchain ().GetImageFormat ()));
            inheritedImages.push_back (InheritedImage::Create (
                swapChainImages[i],
                swapchainProv.GetSwapchain ().GetWidth (),
                swapchainProv.GetSwapchain ().GetHeight (),
                1,
                swapchainProv.GetSwapchain ().GetImageFormat (),
                1));
        }
    }

    // overriding ImageResource
    virtual void          BindRead (uint32_t, CommandBuffer&) override {}
    virtual void          BindWrite (uint32_t, CommandBuffer&) override {}
    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; }
    virtual VkFormat      GetFormat () const override { return swapchainProv.GetSwapchain ().GetImageFormat (); }
    virtual uint32_t      GetDescriptorCount () const override { return 1; }

    virtual std::vector<ImageBase*> GetImages () const override
    {
        std::vector<ImageBase*> result;

        for (auto& img : inheritedImages) {
            result.push_back (img.get ());
        }

        return result;
    }


    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t frameIndex, uint32_t) override { return *imageViews[frameIndex]; }
    virtual VkSampler   GetSampler () override { return VK_NULL_HANDLE; }
};


USING_PTR (CPUBufferResource);

class GEARSVK_API CPUBufferResource : public InputBufferBindableResource {
public:
    const uint32_t              size;
    std::vector<BufferU>        buffers;
    std::vector<MemoryMappingU> mappings;

protected:
    CPUBufferResource (uint32_t size)
        : size (size)
    {
    }

public:
    USING_CREATE (CPUBufferResource);

    virtual ~CPUBufferResource () = default;

    // overriding Resource
    virtual void Compile (const GraphSettings& graphSettings) override
    {
        buffers.clear ();
        mappings.clear ();

        for (uint32_t i = 0; i < graphSettings.framesInFlight; ++i) {
            buffers.push_back (UniformBuffer::Create (graphSettings.GetDevice ().GetAllocator (), size, 0, Buffer::MemoryLocation::CPU));
            mappings.push_back (MemoryMapping::Create (graphSettings.GetDevice ().GetAllocator (), *buffers[buffers.size () - 1]));
        }
    }

    // overriding InputBufferBindable
    virtual VkBuffer GetBufferForFrame (uint32_t frameIndex) override { return *buffers[frameIndex]; }
    virtual uint32_t GetBufferSize () override { return size; }

    MemoryMapping& GetMapping (uint32_t frameIndex) { return *mappings[frameIndex]; }
};

class GEARSVK_API ResourceVisitor final {
public:
    Event<WritableImageResource&>     onWritableImage;
    Event<ReadOnlyImageResource&>     onReadOnlyImage;
    Event<SwapchainImageResource&>    onSwapchainImage;
    Event<CPUBufferResource&>         onCPUBuffer;

    void Visit (Resource& res);

    void Visit (Resource* res)
    {
        Visit (*res);
    }

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