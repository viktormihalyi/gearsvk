#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "RenderGraph/RenderGraphAPI.hpp"

#include "Utils/Event.hpp"
#include "Utils/Timer.hpp"
#include "VulkanWrapper/Utils/VulkanUtils.hpp"
#include "VulkanWrapper/VulkanWrapper.hpp"
#include "VulkanWrapper/Commands.hpp"
#include "VulkanWrapper/Event.hpp"
#include "RenderGraph/ShaderPipeline.hpp"

#include "Connections.hpp"
#include "GraphSettings.hpp"
#include "InputBindable.hpp"
#include "Node.hpp"
#include <memory>

namespace RG {

class GraphSettings;

class Operation;

class IResourceVisitor;

class GVK_RENDERER_API Resource : public Node {
public:
    virtual ~Resource () = default;

    virtual void Compile (const GraphSettings&) = 0;

    virtual void OnPreRead (uint32_t resourceIndex, GVK::CommandBuffer&) {};
    virtual void OnPreWrite (uint32_t resourceIndex, GVK::CommandBuffer&) {};
    virtual void OnPostWrite (uint32_t resourceIndex, GVK::CommandBuffer&) {};
    virtual void OnGraphExecutionStarted (uint32_t resourceIndex, GVK::CommandBuffer&) {};
    virtual void OnGraphExecutionEnded (uint32_t resourceIndex, GVK::CommandBuffer&) {};

    virtual void Visit (IResourceVisitor&) = 0;
};

class ReadOnlyImageResource;
class WritableImageResource;
class SwapchainImageResource;
class GPUBufferResource;
class CPUBufferResource;


class GVK_RENDERER_API IResourceVisitor {
public:
    virtual ~IResourceVisitor () = default;

    virtual void Visit (ReadOnlyImageResource&)  = 0;
    virtual void Visit (WritableImageResource&)  = 0;
    virtual void Visit (SwapchainImageResource&) = 0;
    virtual void Visit (GPUBufferResource&)      = 0;
    virtual void Visit (CPUBufferResource&)      = 0;
};


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


class GVK_RENDERER_API InputBufferBindableResource : public Resource, public InputBufferBindable {
public:
    virtual ~InputBufferBindableResource () = default;
};


class GVK_RENDERER_API ImageResource : public Resource {
public:
    virtual ~ImageResource () = default;

public:
    virtual VkImageLayout       GetInitialLayout () const             = 0;
    virtual VkImageLayout       GetFinalLayout () const               = 0;
    virtual VkFormat            GetFormat () const                    = 0;
    virtual uint32_t            GetLayerCount () const                = 0;
    virtual std::vector<GVK::Image*> GetImages () const                    = 0;
    virtual std::vector<GVK::Image*> GetImages (uint32_t resourceIndex) const = 0;


    std::function<VkFormat ()> GetFormatProvider () const
    {
        return [&] () { return GetFormat (); };
    }
};


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


class GVK_RENDERER_API WritableImageResource : public ImageResource, public InputImageBindable {
protected:
    class GVK_RENDERER_API SingleImageResource final {
    public:
        static const VkFormat FormatRGBA;
        static const VkFormat FormatRGB;

        const std::unique_ptr<GVK::Image>         image;
        std::vector<std::unique_ptr<GVK::ImageView2D>> imageViews;
        std::optional<VkImageLayout>              layoutRead;
        std::optional<VkImageLayout>              layoutWrite;

        // write always happens before read
        // NO  read, NO  write: general
        // YES read, NO  write: general -> read
        // NO  read, YES write: general -> write
        // YES read, YES write: general -> write -> read

        SingleImageResource (const GVK::DeviceExtra& device, uint32_t width, uint32_t height, uint32_t arrayLayers, VkFormat format = FormatRGBA, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL)
            : image (std::make_unique<GVK::Image2D> (device.GetAllocator (), GVK::Image::MemoryLocation::GPU,
                                                width, height,
                                                format, tiling,
                                                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                arrayLayers))
        {
            for (uint32_t layerIndex = 0; layerIndex < arrayLayers; ++layerIndex) {
                imageViews.push_back (std::make_unique<GVK::ImageView2D> (device, *image, layerIndex));
            }

            {
                GVK::SingleTimeCommand s (device);
                s.Record<GVK::CommandTranstionImage> (*image, GVK::Image::INITIAL_LAYOUT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            }
        }

        std::vector<GVK::ImageView2D> CreateImageViews (const GVK::DeviceExtra& device) const
        {
            std::vector<GVK::ImageView2D> result;
            for (uint32_t layerIndex = 0; layerIndex < image->GetArrayLayers (); ++layerIndex) {
                result.emplace_back (device, *image, layerIndex);
            }
            return result;
        }
    };

public:
    const VkFilter filter;
    const VkFormat format;
    const uint32_t width;
    const uint32_t height;
    const uint32_t arrayLayers;

    std::vector<std::unique_ptr<SingleImageResource>> images;
    std::unique_ptr<GVK::Sampler>                     sampler;

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
        sampler = std::make_unique<GVK::Sampler> (graphSettings.GetDevice (), filter);

        images.clear ();
        for (uint32_t resourceIndex = 0; resourceIndex < graphSettings.framesInFlight; ++resourceIndex) {
            images.push_back (std::make_unique<SingleImageResource> (graphSettings.GetDevice (), width, height, arrayLayers, format));
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

    virtual std::vector<GVK::Image*> GetImages () const override
    {
        std::vector<GVK::Image*> result;

        for (auto& img : images) {
            result.push_back (img->image.get ());
        }

        return result;
    }

    virtual std::vector<GVK::Image*> GetImages (uint32_t resourceIndex) const override
    {
        return { images[resourceIndex]->image.get () };
    }
    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t resourceIndex, uint32_t layerIndex) override { return *images[resourceIndex]->imageViews[layerIndex]; }
    virtual VkSampler   GetSampler () override { return *sampler; }

    virtual void Visit (IResourceVisitor& visitor) override { visitor.Visit (*this); }
};


class GVK_RENDERER_API SingleWritableImageResource : public WritableImageResource {
private:
    std::unique_ptr<VW::Event> readWriteSync;

public:
    using WritableImageResource::WritableImageResource;

    virtual void Compile (const GraphSettings& graphSettings) override
    {
        readWriteSync = std::make_unique<VW::Event> (graphSettings.GetDevice ());

        sampler = std::make_unique<GVK::Sampler> (graphSettings.GetDevice (), filter);
        images.clear ();
        images.push_back (std::make_unique<SingleImageResource> (graphSettings.GetDevice (), width, height, arrayLayers, GetFormat ()));
    }

    virtual void OnPreRead (uint32_t resourceIndex, GVK::CommandBuffer& commandBuffer) override
    {
        commandBuffer.Record<GVK::CommandGeneric> ([&] (VkCommandBuffer commandBuffer) {
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

    virtual void OnPreWrite (uint32_t resourceIndex, GVK::CommandBuffer& commandBuffer) override
    {
        commandBuffer.Record<GVK::CommandGeneric> ([&] (VkCommandBuffer commandBuffer) {
            VkEvent handle = *readWriteSync;
            vkCmdResetEvent (commandBuffer, handle, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
        });
    }

    virtual void OnPostWrite (uint32_t resourceIndex, GVK::CommandBuffer& commandBuffer) override
    {
        commandBuffer.Record<GVK::CommandGeneric> ([&] (VkCommandBuffer commandBuffer) {
            VkEvent handle = *readWriteSync;
            vkCmdSetEvent (commandBuffer, handle, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
        });
    }

    virtual void Visit (IResourceVisitor& visitor) override { visitor.Visit (*this); }
};


class GVK_RENDERER_API GPUBufferResource : public InputBufferBindableResource {
private:
    std::unique_ptr<GVK::BufferTransferable> buffer;
    const uint32_t                      size;

public:
    GPUBufferResource (const uint32_t size)
        : size (size)
    {
    }

    virtual ~GPUBufferResource () = default;

    virtual void Compile (const GraphSettings& settings) override
    {
        buffer = std::make_unique<GVK::BufferTransferable> (settings.GetDevice (), size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
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


class GVK_RENDERER_API ReadOnlyImageResource : public OneTimeCompileResource, public InputImageBindable {
public:
    std::unique_ptr<GVK::ImageTransferable> image;
    std::unique_ptr<GVK::ImageViewBase>     imageView;
    std::unique_ptr<GVK::Sampler>           sampler;

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
        sampler = std::make_unique<GVK::Sampler> (settings.GetDevice (), filter);

        if (height == 1 && depth == 1) {
            image     = std::make_unique<GVK::Image1DTransferable> (settings.GetDevice (), format, width, VK_IMAGE_USAGE_SAMPLED_BIT);
            imageView = std::make_unique<GVK::ImageView1D> (settings.GetDevice (), *image->imageGPU);
        } else if (depth == 1) {
            if (layerCount == 1) {
                image     = std::make_unique<GVK::Image2DTransferable> (settings.GetDevice (), format, width, height, VK_IMAGE_USAGE_SAMPLED_BIT);
                imageView = std::make_unique<GVK::ImageView2D> (settings.GetDevice (), *image->imageGPU, 0);
            } else {
                image     = std::make_unique<GVK::Image2DTransferable> (settings.GetDevice (), format, width, height, VK_IMAGE_USAGE_SAMPLED_BIT, layerCount);
                imageView = std::make_unique<GVK::ImageView2DArray> (settings.GetDevice (), *image->imageGPU, 0, layerCount);
            }
        } else {
            image     = std::make_unique<GVK::Image3DTransferable> (settings.GetDevice (), format, width, height, depth, VK_IMAGE_USAGE_SAMPLED_BIT);
            imageView = std::make_unique<GVK::ImageView3D> (settings.GetDevice (), *image->imageGPU);
        }

        GVK::SingleTimeCommand s (settings.GetDevice ());
        s.Record<GVK::CommandTranstionImage> (*image->imageGPU, GVK::Image::INITIAL_LAYOUT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    // overriding ImageResource
    virtual VkImageLayout GetInitialLayout () const override { return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; }
    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; }
    virtual VkFormat      GetFormat () const override { return format; }
    virtual uint32_t      GetLayerCount () const override { return 1; }

    virtual std::vector<GVK::Image*> GetImages () const override
    {
        return { image->imageGPU.get () };
    }

    virtual std::vector<GVK::Image*> GetImages (uint32_t) const override
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


class GVK_RENDERER_API SwapchainImageResource : public ImageResource, public InputImageBindable {
public:
    std::vector<std::unique_ptr<GVK::ImageView2D>> imageViews;
    GVK::SwapchainProvider&                           swapchainProv;
    std::vector<std::unique_ptr<GVK::InheritedImage>> inheritedImages;

public:
    SwapchainImageResource (GVK::SwapchainProvider& swapchainProv)
        : swapchainProv (swapchainProv)
    {
    }

    virtual ~SwapchainImageResource () = default;

    // overriding Resource
    virtual void Compile (const GraphSettings& graphSettings) override
    {
        const std::vector<VkImage> swapChainImages = swapchainProv.GetSwapchain ().GetImages ();

        imageViews.clear ();
        inheritedImages.clear ();

        for (size_t i = 0; i < swapChainImages.size (); ++i) {
            imageViews.push_back (std::make_unique<GVK::ImageView2D> (graphSettings.GetDevice (), swapChainImages[i], swapchainProv.GetSwapchain ().GetImageFormat ()));
            inheritedImages.push_back (std::make_unique<GVK::InheritedImage> (
                swapChainImages[i],
                swapchainProv.GetSwapchain ().GetWidth (),
                swapchainProv.GetSwapchain ().GetHeight (),
                1,
                swapchainProv.GetSwapchain ().GetImageFormat (),
                1));
        }
    }

    std::vector<GVK::ImageView2D> CreateImageViews (const GVK::DeviceExtra& device) const
    {
        const std::vector<VkImage> swapChainImages = swapchainProv.GetSwapchain ().GetImages ();

        std::vector<GVK::ImageView2D> result;
        for (size_t i = 0; i < swapChainImages.size (); ++i) {
            result.emplace_back (device, swapChainImages[i], swapchainProv.GetSwapchain ().GetImageFormat ());
        }
        return result;
    }

    // overriding ImageResource
    virtual VkImageLayout GetInitialLayout () const override { return VK_IMAGE_LAYOUT_UNDEFINED; }
    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; }
    virtual VkFormat      GetFormat () const override { return swapchainProv.GetSwapchain ().GetImageFormat (); }
    virtual uint32_t      GetLayerCount () const override { return 1; }

    virtual std::vector<GVK::Image*> GetImages () const override
    {
        std::vector<GVK::Image*> result;

        for (auto& img : inheritedImages) {
            result.push_back (img.get ());
        }

        return result;
    }

    virtual std::vector<GVK::Image*> GetImages (uint32_t resourceIndex) const override
    {
        return { inheritedImages[resourceIndex].get () };
    }


    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t resourceIndex, uint32_t) override { return *imageViews[resourceIndex]; }
    virtual VkSampler   GetSampler () override { return VK_NULL_HANDLE; }

    virtual void Visit (IResourceVisitor& visitor) override { visitor.Visit (*this); }
};


class GVK_RENDERER_API CPUBufferResource : public InputBufferBindableResource {
public:
    const uint32_t                              size;
    std::vector<std::unique_ptr<GVK::Buffer>>        buffers;
    std::vector<std::unique_ptr<GVK::MemoryMapping>> mappings;

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
        mappings.clear ();
        buffers.clear ();

        for (uint32_t i = 0; i < graphSettings.framesInFlight; ++i) {
            buffers.push_back (std::make_unique<GVK::UniformBuffer> (graphSettings.GetDevice ().GetAllocator (), size, 0, GVK::Buffer::MemoryLocation::CPU));
            mappings.push_back (std::make_unique<GVK::MemoryMapping> (graphSettings.GetDevice ().GetAllocator (), *buffers[buffers.size () - 1]));
        }
    }

    // overriding InputBufferBindable
    virtual VkBuffer GetBufferForFrame (uint32_t resourceIndex) override { return *buffers[resourceIndex]; }
    virtual uint32_t GetBufferSize () override { return size; }

    GVK::MemoryMapping& GetMapping (uint32_t resourceIndex) { return *mappings[resourceIndex]; }

    virtual void Visit (IResourceVisitor& visitor) override { visitor.Visit (*this); }
};


} // namespace RG

#endif