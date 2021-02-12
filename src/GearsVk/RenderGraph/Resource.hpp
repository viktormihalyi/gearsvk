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

namespace GVK {

namespace RG {

class GraphSettings;

class Operation;

class IResourceVisitor;

USING_PTR (Resource);
class GVK_RENDERER_API Resource : public Node {
public:
    virtual ~Resource () = default;

    virtual void Compile (const GraphSettings&) = 0;

    virtual void OnPreRead (uint32_t frameIndex, CommandBuffer&) {};
    virtual void OnPreWrite (uint32_t frameIndex, CommandBuffer&) {};
    virtual void OnPostWrite (uint32_t frameIndex, CommandBuffer&) {};
    virtual void OnGraphExecutionStarted (uint32_t frameIndex, CommandBuffer&) {};
    virtual void OnGraphExecutionEnded (uint32_t frameIndex, CommandBuffer&) {};

    virtual void Visit (IResourceVisitor&) = 0;
};

class ReadOnlyImageResource;
class WritableImageResource;
class SwapchainImageResource;
class GPUBufferResource;
class CPUBufferResource;


USING_PTR (IResourceVisitor);
class GVK_RENDERER_API IResourceVisitor {
public:
    virtual ~IResourceVisitor () = default;

    virtual void Visit (ReadOnlyImageResource&)  = 0;
    virtual void Visit (WritableImageResource&)  = 0;
    virtual void Visit (SwapchainImageResource&) = 0;
    virtual void Visit (GPUBufferResource&)      = 0;
    virtual void Visit (CPUBufferResource&)      = 0;
};


USING_PTR (IResourceVisitorFn);
class IResourceVisitorFn : public IResourceVisitor {
private:
    template<typename T>
    using VisitorFn = std::function<void (T&)>;

    VisitorFn<ReadOnlyImageResource>  r1;
    VisitorFn<WritableImageResource>  r2;
    VisitorFn<SwapchainImageResource> r3;
    VisitorFn<GPUBufferResource>      r4;
    VisitorFn<CPUBufferResource>      r5;

public:
    IResourceVisitorFn (const VisitorFn<ReadOnlyImageResource>&  r1,
                        const VisitorFn<WritableImageResource>&  r2,
                        const VisitorFn<SwapchainImageResource>& r3,
                        const VisitorFn<GPUBufferResource>&      r4,
                        const VisitorFn<CPUBufferResource>&      r5)
        : r1 (r1)
        , r2 (r2)
        , r3 (r3)
        , r4 (r4)
        , r5 (r5)
    {
    }

    virtual void Visit (ReadOnlyImageResource& resource) override { r1 (resource); }
    virtual void Visit (WritableImageResource& resource) override { r2 (resource); }
    virtual void Visit (SwapchainImageResource& resource) override { r3 (resource); }
    virtual void Visit (GPUBufferResource& resource) override { r4 (resource); }
    virtual void Visit (CPUBufferResource& resource) override { r5 (resource); }
};


USING_PTR (InputBufferBindableResource);
class GVK_RENDERER_API InputBufferBindableResource : public Resource, public InputBufferBindable {
public:
    virtual ~InputBufferBindableResource () = default;
};


USING_PTR (ImageResource);
class GVK_RENDERER_API ImageResource : public Resource {
public:
    virtual ~ImageResource () = default;

public:
    virtual VkImageLayout       GetInitialLayout () const             = 0;
    virtual VkImageLayout       GetFinalLayout () const               = 0;
    virtual VkFormat            GetFormat () const                    = 0;
    virtual uint32_t            GetLayerCount () const                = 0;
    virtual std::vector<Image*> GetImages () const                    = 0;
    virtual std::vector<Image*> GetImages (uint32_t frameIndex) const = 0;

    std::function<VkFormat ()> GetFormatProvider () const
    {
        return [&] () { return GetFormat (); };
    }
};


USING_PTR (OneTimeCompileResource);
class GVK_RENDERER_API OneTimeCompileResource : public ImageResource {
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
class GVK_RENDERER_API WritableImageResource : public ImageResource, public InputImageBindable {
protected:
    USING_PTR (SingleImageResource);
    class GVK_RENDERER_API SingleImageResource final {
    public:
        static const VkFormat FormatRGBA;
        static const VkFormat FormatRGB;

        const ImageU                 image;
        std::vector<ImageView2DU>    imageViews;
        std::optional<VkImageLayout> layoutRead;
        std::optional<VkImageLayout> layoutWrite;

        // write always happens before read
        // NO  read, NO  write: general
        // YES read, NO  write: general -> read
        // NO  read, YES write: general -> write
        // YES read, YES write: general -> write -> read

        SingleImageResource (const DeviceExtra& device, uint32_t width, uint32_t height, uint32_t arrayLayers, VkFormat format = FormatRGBA, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL)
            : image (Make<Image2D> (device.GetAllocator (), Image::MemoryLocation::GPU,
                                    width, height,
                                    format, tiling,
                                    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                    arrayLayers))
        {
            for (uint32_t layerIndex = 0; layerIndex < arrayLayers; ++layerIndex) {
                imageViews.push_back (Make<ImageView2D> (device, *image, layerIndex));
            }

            {
                SingleTimeCommand s (device);
                image->CmdPipelineBarrier (s, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            }
        }
    };

public:
    const VkFilter filter;
    const VkFormat format;
    const uint32_t width;
    const uint32_t height;
    const uint32_t arrayLayers;

    std::vector<SingleImageResourceU> images;
    SamplerU                          sampler;

public:
    WritableImageResource (VkFilter filter, uint32_t width, uint32_t height, uint32_t arrayLayers, VkFormat format = SingleImageResource::FormatRGBA)
        : filter (filter)
        , format (format)
        , width (width)
        , height (height)
        , arrayLayers (arrayLayers)
    {
    }

    WritableImageResource (uint32_t width, uint32_t height)
        : WritableImageResource (VK_FILTER_LINEAR, width, height, 1)
    {
    }

    virtual ~WritableImageResource () = default;

    virtual void Compile (const GraphSettings& graphSettings) override
    {
        sampler = Make<Sampler> (graphSettings.GetDevice (), filter);

        images.clear ();
        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            images.push_back (Make<SingleImageResource> (graphSettings.GetDevice (), width, height, arrayLayers, format));
        }
    }

    // overriding ImageResource
    virtual VkImageLayout GetInitialLayout () const override
    {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    virtual VkImageLayout GetFinalLayout () const override
    {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    virtual VkFormat GetFormat () const override { return format; }
    virtual uint32_t GetLayerCount () const override { return arrayLayers; }

    virtual std::vector<Image*> GetImages () const override
    {
        std::vector<Image*> result;

        for (auto& img : images) {
            result.push_back (img->image.get ());
        }

        return result;
    }

    virtual std::vector<Image*> GetImages (uint32_t frameIndex) const override
    {
        return { images[frameIndex]->image.get () };
    }
    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t frameIndex, uint32_t layerIndex) override { return *images[frameIndex]->imageViews[layerIndex]; }
    virtual VkSampler   GetSampler () override { return *sampler; }

    virtual void Visit (IResourceVisitor& visitor) override { visitor.Visit (*this); }
};

namespace GVKr {

USING_PTR (Event);
class Event : public VulkanObject {
private:
    VkDevice device;
    VkEvent  handle;

public:
    Event (VkDevice device)
        : device (device)
    {
        VkEventCreateInfo createInfo = {};
        createInfo.sType             = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        createInfo.pNext             = nullptr;
        createInfo.flags             = 0;

        if (GVK_ERROR (vkCreateEvent (device, &createInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create vkevent");
        }
    }

    virtual ~Event () override
    {
        vkDestroyEvent (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkEvent () const { return handle; }
};

} // namespace GVKr


USING_PTR (SingleWritableImageResource);
class GVK_RENDERER_API SingleWritableImageResource : public WritableImageResource {
private:
    GVKr::EventU readWriteSync;

public:
    using WritableImageResource::WritableImageResource;

    virtual void Compile (const GraphSettings& graphSettings) override
    {
        readWriteSync = Make<GVKr::Event> (graphSettings.GetDevice ());

        sampler = Make<Sampler> (graphSettings.GetDevice (), filter);
        images.clear ();
        images.push_back (Make<SingleImageResource> (graphSettings.GetDevice (), width, height, arrayLayers, GetFormat ()));
    }

    virtual void OnPreRead (uint32_t frameIndex, CommandBuffer& commandBuffer) override
    {
        commandBuffer.RecordT<CommandGeneric> ([&] (VkCommandBuffer commandBuffer) {
            VkEvent handle = *readWriteSync;
            vkCmdWaitEvents (commandBuffer,
                             1,
                             &handle,
                             VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                             VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                             0, nullptr,
                             0, nullptr,
                             0, nullptr);
        });
    }

    virtual void OnPreWrite (uint32_t frameIndex, CommandBuffer& commandBuffer) override
    {
        commandBuffer.RecordT<CommandGeneric> ([&] (VkCommandBuffer commandBuffer) {
            vkCmdResetEvent (commandBuffer, *readWriteSync, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
        });
    }

    virtual void OnPostWrite (uint32_t frameIndex, CommandBuffer& commandBuffer) override
    {
        commandBuffer.RecordT<CommandGeneric> ([&] (VkCommandBuffer commandBuffer) {
            vkCmdSetEvent (commandBuffer, *readWriteSync, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
        });
    }

    virtual void Visit (IResourceVisitor& visitor) override { visitor.Visit (*this); }
};


USING_PTR (GPUBufferResource);
class GVK_RENDERER_API GPUBufferResource : public InputBufferBindableResource {
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
        buffer = Make<BufferTransferable> (settings.GetDevice (), size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
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

    virtual void Visit (IResourceVisitor& visitor) override { visitor.Visit (*this); }
};


USING_PTR (ReadOnlyImageResource);
class GVK_RENDERER_API ReadOnlyImageResource : public OneTimeCompileResource, public InputImageBindable {
public:
    ImageTransferableU image;
    ImageViewBaseU     imageView;
    SamplerU           sampler;

    const VkFormat format;
    const VkFilter filter;
    const uint32_t width;
    const uint32_t height;
    const uint32_t depth;
    const uint32_t layerCount;

public:
    ReadOnlyImageResource (VkFormat format, VkFilter filter, uint32_t width, uint32_t height = 1, uint32_t depth = 1, uint32_t layerCount = 1)
        : format (format)
        , filter (filter)
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

    ReadOnlyImageResource (VkFormat format, uint32_t width, uint32_t height = 1, uint32_t depth = 1, uint32_t layerCount = 1)
        : ReadOnlyImageResource (format, VK_FILTER_LINEAR, width, height, depth, layerCount)
    {
    }

    virtual ~ReadOnlyImageResource () {}

    // overriding OneTimeCompileResource
    virtual void CompileOnce (const GraphSettings& settings) override
    {
        sampler = Make<Sampler> (settings.GetDevice (), filter);

        if (height == 1 && depth == 1) {
            image     = Make<Image1DTransferable> (settings.GetDevice (), format, width, VK_IMAGE_USAGE_SAMPLED_BIT);
            imageView = Make<ImageView1D> (settings.GetDevice (), *image->imageGPU);
        } else if (depth == 1) {
            if (layerCount == 1) {
                image     = Make<Image2DTransferable> (settings.GetDevice (), format, width, height, VK_IMAGE_USAGE_SAMPLED_BIT);
                imageView = Make<ImageView2D> (settings.GetDevice (), *image->imageGPU, 0);
            } else {
                image     = Make<Image2DTransferable> (settings.GetDevice (), format, width, height, VK_IMAGE_USAGE_SAMPLED_BIT, layerCount);
                imageView = Make<ImageView2DArray> (settings.GetDevice (), *image->imageGPU, 0, layerCount);
            }
        } else {
            image     = Make<Image3DTransferable> (settings.GetDevice (), format, width, height, depth, VK_IMAGE_USAGE_SAMPLED_BIT);
            imageView = Make<ImageView3D> (settings.GetDevice (), *image->imageGPU);
        }

        SingleTimeCommand s (settings.GetDevice ());
        image->imageGPU->CmdPipelineBarrier (s, Image::INITIAL_LAYOUT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    // overriding ImageResource
    virtual VkImageLayout GetInitialLayout () const override { return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; }
    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; }
    virtual VkFormat      GetFormat () const override { return format; }
    virtual uint32_t      GetLayerCount () const override { return 1; }

    virtual std::vector<Image*> GetImages () const override
    {
        return { image->imageGPU.get () };
    }

    virtual std::vector<Image*> GetImages (uint32_t) const override
    {
        return GetImages ();
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

    virtual void Visit (IResourceVisitor& visitor) override { visitor.Visit (*this); }
};


USING_PTR (SwapchainImageResource);
class GVK_RENDERER_API SwapchainImageResource : public ImageResource, public InputImageBindable {
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
            imageViews.push_back (Make<ImageView2D> (graphSettings.GetDevice (), swapChainImages[i], swapchainProv.GetSwapchain ().GetImageFormat ()));
            inheritedImages.push_back (Make<InheritedImage> (
                swapChainImages[i],
                swapchainProv.GetSwapchain ().GetWidth (),
                swapchainProv.GetSwapchain ().GetHeight (),
                1,
                swapchainProv.GetSwapchain ().GetImageFormat (),
                1));
        }
    }

    // overriding ImageResource
    virtual VkImageLayout GetInitialLayout () const override { return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; }
    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; }
    virtual VkFormat      GetFormat () const override { return swapchainProv.GetSwapchain ().GetImageFormat (); }
    virtual uint32_t      GetLayerCount () const override { return 1; }

    virtual std::vector<Image*> GetImages () const override
    {
        std::vector<Image*> result;

        for (auto& img : inheritedImages) {
            result.push_back (img.get ());
        }

        return result;
    }

    virtual std::vector<Image*> GetImages (uint32_t frameIndex) const override
    {
        return { inheritedImages[frameIndex].get () };
    }


    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t frameIndex, uint32_t) override { return *imageViews[frameIndex]; }
    virtual VkSampler   GetSampler () override { return VK_NULL_HANDLE; }

    virtual void Visit (IResourceVisitor& visitor) override { visitor.Visit (*this); }
};


USING_PTR (CPUBufferResource);
class GVK_RENDERER_API CPUBufferResource : public InputBufferBindableResource {
public:
    const uint32_t              size;
    std::vector<BufferU>        buffers;
    std::vector<MemoryMappingU> mappings;

public:
    CPUBufferResource (uint32_t size)
        : size (size)
    {
    }

public:
    virtual ~CPUBufferResource () = default;

    // overriding Resource
    virtual void Compile (const GraphSettings& graphSettings) override
    {
        buffers.clear ();
        mappings.clear ();

        for (uint32_t i = 0; i < graphSettings.framesInFlight; ++i) {
            buffers.push_back (Make<UniformBuffer> (graphSettings.GetDevice ().GetAllocator (), size, 0, Buffer::MemoryLocation::CPU));
            mappings.push_back (Make<MemoryMapping> (graphSettings.GetDevice ().GetAllocator (), *buffers[buffers.size () - 1]));
        }
    }

    // overriding InputBufferBindable
    virtual VkBuffer GetBufferForFrame (uint32_t frameIndex) override { return *buffers[frameIndex]; }
    virtual uint32_t GetBufferSize () override { return size; }

    MemoryMapping& GetMapping (uint32_t frameIndex) { return *mappings[frameIndex]; }

    virtual void Visit (IResourceVisitor& visitor) override { visitor.Visit (*this); }
};


} // namespace RG

} // namespace GVK

#endif