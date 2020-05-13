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
#include "UniformBlock.hpp"


namespace RG {


class GEARSVK_API Resource : public Node {
public:
    USING_PTR_ABSTRACT (Resource);

    virtual ~Resource () = default;

    virtual void Compile (const GraphSettings&) = 0;
};


class GEARSVK_API ImageResource : public Resource {
public:
    USING_PTR_ABSTRACT (ImageResource);
    virtual ~ImageResource () = default;

    virtual void          BindRead (uint32_t imageIndex, VkCommandBuffer commandBuffer)  = 0;
    virtual void          BindWrite (uint32_t imageIndex, VkCommandBuffer commandBuffer) = 0;
    virtual VkImageLayout GetFinalLayout () const                                        = 0;
    virtual VkFormat      GetFormat () const                                             = 0;
    virtual uint32_t      GetDescriptorCount () const                                    = 0;
};


class GEARSVK_API OneTimeCompileResource : public ImageResource {
private:
    bool compiled;

protected:
    OneTimeCompileResource ()
        : compiled (false)
    {
    }

public:
    USING_PTR_ABSTRACT (OneTimeCompileResource);

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


class GEARSVK_API WritableImageResource : public ImageResource, public InputImageBindable {
private:
    Sampler::U sampler;

    struct SingleImageResource final {
        static const VkFormat FormatRGBA;
        static const VkFormat FormatRGB;

        const AllocatedImage         image;
        std::vector<ImageView2D::U>  imageViews;
        std::optional<VkImageLayout> layoutRead;
        std::optional<VkImageLayout> layoutWrite;

        // write always happens before read
        // NO  read, NO  write: general
        // YES read, NO  write: general -> read
        // NO  read, YES write: general -> write
        // YES read, YES write: general -> write -> read

        USING_PTR (SingleImageResource);

        SingleImageResource (const GraphSettings& graphSettings, uint32_t arrayLayers)
            : image (graphSettings.GetDevice (), Image2D::Create (graphSettings.GetDevice (), graphSettings.width, graphSettings.height, FormatRGBA, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, arrayLayers), DeviceMemory::GPU)
        {
            for (uint32_t layerIndex = 0; layerIndex < arrayLayers; ++layerIndex) {
                imageViews.push_back (ImageView2D::Create (graphSettings.GetDevice (), *image.image, layerIndex));
            }
        }
    };

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

    virtual void BindRead (uint32_t imageIndex, VkCommandBuffer commandBuffer) override
    {
        SingleImageResource& im = *images[imageIndex];

        im.layoutRead                = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkImageLayout previousLayout = (im.layoutWrite.has_value ()) ? *im.layoutWrite : ImageBase::INITIAL_LAYOUT;
        im.image.image->CmdPipelineBarrier (commandBuffer, previousLayout, *im.layoutRead);
    }

    virtual void BindWrite (uint32_t imageIndex, VkCommandBuffer commandBuffer) override
    {
        SingleImageResource& im = *images[imageIndex];

        im.layoutWrite = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        im.image.image->CmdPipelineBarrier (commandBuffer, ImageBase::INITIAL_LAYOUT, *im.layoutWrite);
    }

    virtual void Compile (const GraphSettings& graphSettings)
    {
        sampler = Sampler::Create (graphSettings.GetDevice ());

        images.clear ();
        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            images.push_back (SingleImageResource::Create (graphSettings, arrayLayers));
        }
    }

    // overriding ImageResource
    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; }
    virtual VkFormat      GetFormat () const override { return SingleImageResource::FormatRGBA; }
    virtual uint32_t      GetDescriptorCount () const override { return arrayLayers; }

    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t frameIndex, uint32_t layerIndex) override { return *images[frameIndex]->imageViews[layerIndex]; }
    virtual VkSampler   GetSampler () override { return *sampler; }
};


class GEARSVK_API ReadOnlyImageResource : public OneTimeCompileResource, public InputImageBindable {
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

    // overriding OneTimeCompileResource
    virtual void CompileOnce (const GraphSettings& settings) override
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

    // overriding ImageResource
    virtual void          BindRead (uint32_t, VkCommandBuffer) override {}
    virtual void          BindWrite (uint32_t, VkCommandBuffer) override {}
    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; }
    virtual VkFormat      GetFormat () const override { return format; }
    virtual uint32_t      GetDescriptorCount () const override { return 1; }

    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t, uint32_t) override { return *imageView; }
    virtual VkSampler   GetSampler () override { return *sampler; }

    void CopyTransitionTransfer (const std::vector<uint8_t>& pixelData)
    {
        image->CopyTransitionTransfer (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, pixelData.data (), pixelData.size (), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
};


class GEARSVK_API SwapchainImageResource : public ImageResource, public InputImageBindable {
public:
    std::vector<ImageView2D::U> imageViews;
    Swapchain&                  swapchain;

public:
    USING_PTR (SwapchainImageResource);
    SwapchainImageResource (Swapchain& swapchain)
        : swapchain (swapchain)
    {
    }

    virtual ~SwapchainImageResource () = default;

    // overriding Resource
    virtual void Compile (const GraphSettings& graphSettings) override
    {
        std::vector<VkImage> swapChainImages = swapchain.GetImages ();

        imageViews.clear ();
        for (size_t i = 0; i < swapChainImages.size (); ++i) {
            imageViews.push_back (ImageView2D::Create (graphSettings.GetDevice (), swapChainImages[i], swapchain.GetImageFormat ()));
        }
    }

    // overriding ImageResource
    virtual void          BindRead (uint32_t, VkCommandBuffer) override {}
    virtual void          BindWrite (uint32_t, VkCommandBuffer) override {}
    virtual VkImageLayout GetFinalLayout () const override { return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; }
    virtual VkFormat      GetFormat () const override { return swapchain.GetImageFormat (); }
    virtual uint32_t      GetDescriptorCount () const override { return 1; }

    // overriding InputImageBindable
    virtual VkImageView GetImageViewForFrame (uint32_t frameIndex, uint32_t) override { return *imageViews[frameIndex]; }
    virtual VkSampler   GetSampler () override { return VK_NULL_HANDLE; }
};


class GEARSVK_API CPUBufferResource : public Resource, public InputBufferBindable {
public:
    const uint32_t                  size;
    std::vector<AllocatedBuffer::U> buffers;
    std::vector<MemoryMapping::U>   mappings;

protected:
    CPUBufferResource (uint32_t size)
        : size (size)
    {
    }

public:
    USING_PTR (CPUBufferResource);

    virtual ~CPUBufferResource () = default;

    // overriding Resource
    virtual void Compile (const GraphSettings& graphSettings) override
    {
        for (uint32_t i = 0; i < graphSettings.framesInFlight; ++i) {
            buffers.push_back (AllocatedBuffer::Create (graphSettings.GetDevice (), UniformBuffer::Create (graphSettings.GetDevice (), size), DeviceMemory::CPU));
            mappings.push_back (MemoryMapping::Create (graphSettings.GetDevice (), *buffers[buffers.size () - 1]->memory, 0, size));
        }
    }

    // overriding InputBufferBindable
    virtual VkBuffer GetBufferForFrame (uint32_t frameIndex) override { return *buffers[frameIndex]->buffer; }
    virtual uint32_t GetBufferSize () override { return size; }

    MemoryMapping& GetMapping (uint32_t frameIndex) { return *mappings[frameIndex]; }
};


class GEARSVK_API UniformBlockResource : public CPUBufferResource {
public:
    USING_PTR (UniformBlockResource);

    UniformBlockResource (uint32_t size)
        : CPUBufferResource (size)
    {
    }

    UniformBlockResource (const ShaderStruct& structType)
        : CPUBufferResource (structType.GetFullSize ())
    {
    }

    virtual ~UniformBlockResource () = default;

    void Set (uint32_t frameIndex, UniformBlock& uniformBlock)
    {
        ASSERT (uniformBlock.GetSize () == size);
        GetMapping (frameIndex).Copy (uniformBlock.GetData (), uniformBlock.GetSize (), 0);
    }
};


class GEARSVK_API UniformReflectionResource : public Resource {
public:
    USING_PTR (UniformReflectionResource);

    ShaderPipeline::P& pipeline;

    ShaderBlocks vert;
    ShaderBlocks frag;
    ShaderBlocks geom;
    ShaderBlocks tese;
    ShaderBlocks tesc;
    ShaderBlocks comp;

    std::vector<UniformBlockResource::U> uboRes;
    std::vector<uint32_t>                bindings;
    std::vector<UniformBlock::P>         dataBlocks;

    std::vector<ReadOnlyImageResource::U> sampledImages;
    std::vector<uint32_t>                 samplerBindings;

    enum class Strategy {
        All,
        UniformBlocksOnly,
        SamplersOnly,
    };


    UniformReflectionResource (ShaderPipeline::P& pipeline, Strategy s = Strategy::All)
        : pipeline (pipeline)
    {
        vert.Clear ();
        uboRes.clear ();
        dataBlocks.clear ();

        // TODO move this logic and the input binding connection stuff to a seperate class

        auto GatherFor = [&] (const ShaderModule::U& sm, ShaderBlocks& out) {
            if (sm != nullptr) {
                if (s == Strategy::All || s == Strategy::UniformBlocksOnly) {
                    ShaderBlocks newblocks;
                    for (const auto& s : sm->GetReflection ().ubos) {
                        const ShaderStruct autoStruct (s);

                        auto c = UniformBlockResource::Create (autoStruct);

                        UniformBlock::P autoBlock = UniformBlock::CreateShared (s.binding, s.name, autoStruct);

                        dataBlocks.push_back (autoBlock);
                        bindings.push_back (s.binding);
                        uboRes.push_back (std::move (c));
                        out.AddBlock (autoBlock);
                    }
                }

                if (s == Strategy::All || s == Strategy::SamplersOnly) {
                    for (const auto& s : sm->GetReflection ().samplers) {
                        samplerBindings.push_back (s.binding);
                        if (s.type == SR::Sampler::Type::Sampler1D) {
                            sampledImages.push_back (ReadOnlyImageResource::Create (VK_FORMAT_R8G8B8A8_SRGB, 512));
                        } else if (s.type == SR::Sampler::Type::Sampler2D) {
                            sampledImages.push_back (ReadOnlyImageResource::Create (VK_FORMAT_R8G8B8A8_SRGB, 512, 512));
                        } else if (s.type == SR::Sampler::Type::Sampler3D) {
                            ASSERT (false);
                            sampledImages.push_back (ReadOnlyImageResource::Create (VK_FORMAT_R8_SRGB, 512, 512, 512));
                        } else {
                            ASSERT (false);
                        }
                    }
                }
            }
        };

        GatherFor (pipeline->vertexShader.shader, vert);
        GatherFor (pipeline->fragmentShader.shader, frag);
        GatherFor (pipeline->geometryShader.shader, geom);
        GatherFor (pipeline->tessellationEvaluationShader.shader, tese);
        GatherFor (pipeline->tessellationControlShader.shader, tesc);
        GatherFor (pipeline->computeShader.shader, comp);
    }

    virtual ~UniformReflectionResource () = default;

    virtual void Compile (const GraphSettings& settings)
    {
        for (auto& res : uboRes) {
            res->Compile (settings);
        }
        for (auto& res : sampledImages) {
            res->Compile (settings);
        }
    }

    void Update (uint32_t frameIndex)
    {
        for (uint32_t i = 0; i < uboRes.size (); ++i) {
            uboRes[i]->Set (frameIndex, *dataBlocks[i]);
        }
    }

    template<typename T>
    void SetAll (const std::string& ubo, const std::string& variableName, const T& oss)
    {
        vert[ubo][variableName] = value;
        frag[ubo][variableName] = value;
        geom[ubo][variableName] = value;
        tese[ubo][variableName] = value;
        tesc[ubo][variableName] = value;
        comp[ubo][variableName] = value;
    }
};


class GEARSVK_API ResourceVisitor final {
public:
    Event<WritableImageResource&>     onWritableImage;
    Event<ReadOnlyImageResource&>     onReadOnlyImage;
    Event<SwapchainImageResource&>    onSwapchainImage;
    Event<UniformBlockResource&>      onUniformBlock;
    Event<UniformReflectionResource&> onUniformReflection;

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