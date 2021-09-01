#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "RenderGraph/RenderGraphAPI.hpp"

#include "Utils/Event.hpp"
#include "Utils/Timer.hpp"

#include "VulkanWrapper/Event.hpp"
#include "VulkanWrapper/Pipeline.hpp"

#include "RenderGraph/Connections.hpp"
#include "RenderGraph/GraphSettings.hpp"
#include "RenderGraph/InputBindable.hpp"
#include "RenderGraph/Node.hpp"

#include <vulkan/vulkan.h>

#include <optional>


namespace VW {
class Event;
}

namespace GVK {
class SwapchainProvider;
class Image;
class ImageView2D;
class MemoryMapping;
class Buffer;
class ImageViewBase;
class Sampler;
class ImageTransferable;
class BufferTransferable;
class InheritedImage;
}

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
    virtual ~ImageResource ();

public:
    virtual VkImageLayout            GetInitialLayout () const                = 0;
    virtual VkImageLayout            GetFinalLayout () const                  = 0;
    virtual VkFormat                 GetFormat () const                       = 0;
    virtual uint32_t                 GetLayerCount () const                   = 0;
    virtual std::vector<GVK::Image*> GetImages () const                       = 0;
    virtual std::vector<GVK::Image*> GetImages (uint32_t resourceIndex) const = 0;


    std::function<VkFormat ()> GetFormatProvider () const;
};


class GVK_RENDERER_API OneTimeCompileResource : public ImageResource {
private:
    bool compiled;

protected:
    OneTimeCompileResource ();

public:
    virtual ~OneTimeCompileResource ();

    void Compile (const GraphSettings& settings) override;

    virtual void CompileOnce (const GraphSettings&) = 0;
};


class GVK_RENDERER_API WritableImageResource : public ImageResource, public InputImageBindable {
protected:
    class GVK_RENDERER_API SingleImageResource final {
    public:
        static const VkFormat FormatRGBA;
        static const VkFormat FormatRGB;

        const std::unique_ptr<GVK::Image>              image;
        std::vector<std::unique_ptr<GVK::ImageView2D>> imageViews;
        std::optional<VkImageLayout>                   layoutRead;
        std::optional<VkImageLayout>                   layoutWrite;

        // write always happens before read
        // NO  read, NO  write: general
        // YES read, NO  write: general -> read
        // NO  read, YES write: general -> write
        // YES read, YES write: general -> write -> read

        SingleImageResource (const GVK::DeviceExtra& device, uint32_t width, uint32_t height, uint32_t arrayLayers, VkFormat format = FormatRGBA, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL);

        std::vector<GVK::ImageView2D> CreateImageViews (const GVK::DeviceExtra& device) const;
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
    WritableImageResource (VkFilter filter, uint32_t width, uint32_t height, uint32_t arrayLayers, VkFormat format = SingleImageResource::FormatRGBA);

    WritableImageResource (uint32_t width, uint32_t height);

    virtual ~WritableImageResource ();

    virtual void Compile (const GraphSettings& graphSettings) override;

    // overriding ImageResource
    virtual VkImageLayout GetInitialLayout () const override;

    virtual VkImageLayout GetFinalLayout () const override;

    virtual VkFormat GetFormat () const override;
    virtual uint32_t GetLayerCount () const override;

    virtual std::vector<GVK::Image*> GetImages () const override;

    virtual std::vector<GVK::Image*> GetImages (uint32_t resourceIndex) const override;

    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t resourceIndex, uint32_t layerIndex) override;
    virtual VkSampler   GetSampler () override;

    virtual void Visit (IResourceVisitor& visitor) override;
};


class GVK_RENDERER_API SingleWritableImageResource : public WritableImageResource {
private:
    std::unique_ptr<VW::Event> readWriteSync;

public:
    using WritableImageResource::WritableImageResource;

    virtual void Compile (const GraphSettings& graphSettings) override;

    virtual void OnPreRead (uint32_t resourceIndex, GVK::CommandBuffer& commandBuffer) override;

    virtual void OnPreWrite (uint32_t resourceIndex, GVK::CommandBuffer& commandBuffer) override;

    virtual void OnPostWrite (uint32_t resourceIndex, GVK::CommandBuffer& commandBuffer) override;

    virtual void Visit (IResourceVisitor& visitor) override;
};


class GVK_RENDERER_API GPUBufferResource : public InputBufferBindableResource {
private:
    std::unique_ptr<GVK::BufferTransferable> buffer;
    const uint32_t                           size;

public:
    GPUBufferResource (const uint32_t size);

    virtual ~GPUBufferResource ();

    virtual void Compile (const GraphSettings& settings) override;

    // overriding InputImageBindable
    virtual VkBuffer GetBufferForFrame (uint32_t) override;

    virtual uint32_t GetBufferSize () override;

    void CopyAndTransfer (const void* data, size_t size) const;

    virtual void Visit (IResourceVisitor& visitor) override;
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
    ReadOnlyImageResource (VkFormat format, VkFilter filter, uint32_t width, uint32_t height = 1, uint32_t depth = 1, uint32_t layerCount = 1);

    ReadOnlyImageResource (VkFormat format, uint32_t width, uint32_t height = 1, uint32_t depth = 1, uint32_t layerCount = 1);

    virtual ~ReadOnlyImageResource () override;

    // overriding OneTimeCompileResource
    virtual void CompileOnce (const GraphSettings& settings) override;

    // overriding ImageResource
    virtual VkImageLayout GetInitialLayout () const override;
    virtual VkImageLayout GetFinalLayout () const override;
    virtual VkFormat      GetFormat () const override;
    virtual uint32_t      GetLayerCount () const override;

    virtual std::vector<GVK::Image*> GetImages () const override;

    virtual std::vector<GVK::Image*> GetImages (uint32_t) const override;

    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t, uint32_t) override;
    virtual VkSampler   GetSampler () override;

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

    virtual void Visit (IResourceVisitor& visitor) override;
};


class GVK_RENDERER_API SwapchainImageResource : public ImageResource, public InputImageBindable {
public:
    std::vector<std::unique_ptr<GVK::ImageView2D>>    imageViews;
    GVK::SwapchainProvider&                           swapchainProv;
    std::vector<std::unique_ptr<GVK::InheritedImage>> inheritedImages;

public:
    SwapchainImageResource (GVK::SwapchainProvider& swapchainProv);

    virtual ~SwapchainImageResource ();

    // overriding Resource
    virtual void Compile (const GraphSettings& graphSettings) override;

    std::vector<GVK::ImageView2D> CreateImageViews (const GVK::DeviceExtra& device) const;

    // overriding ImageResource
    virtual VkImageLayout GetInitialLayout () const override;
    virtual VkImageLayout GetFinalLayout () const override;
    virtual VkFormat      GetFormat () const override;
    virtual uint32_t      GetLayerCount () const override;

    virtual std::vector<GVK::Image*> GetImages () const override;

    virtual std::vector<GVK::Image*> GetImages (uint32_t resourceIndex) const override;

    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t resourceIndex, uint32_t) override;
    virtual VkSampler   GetSampler () override;

    virtual void Visit (IResourceVisitor& visitor) override;
};


class GVK_RENDERER_API CPUBufferResource : public InputBufferBindableResource {
public:
    const uint32_t                                   size;
    std::vector<std::unique_ptr<GVK::Buffer>>        buffers;
    std::vector<std::unique_ptr<GVK::MemoryMapping>> mappings;

public:
    CPUBufferResource (uint32_t size);

public:
    virtual ~CPUBufferResource ();

    // overriding Resource
    virtual void Compile (const GraphSettings& graphSettings) override;

    // overriding InputBufferBindable
    virtual VkBuffer GetBufferForFrame (uint32_t resourceIndex) override;
    virtual uint32_t GetBufferSize () override;

    GVK::MemoryMapping& GetMapping (uint32_t resourceIndex);

    virtual void Visit (IResourceVisitor& visitor) override;
};


} // namespace RG

#endif