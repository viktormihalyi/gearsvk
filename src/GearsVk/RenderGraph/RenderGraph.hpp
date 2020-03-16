#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP

#include "Shaderpipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

namespace RenderGraph {

struct GraphInfo {
    uint32_t width;
    uint32_t height;
};

struct InputBinding {
    const uint32_t               binding;
    VkDescriptorSetLayoutBinding descriptor;

    InputBinding (uint32_t binding)
        : binding (binding)
    {
        descriptor.binding            = binding;
        descriptor.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor.descriptorCount    = 1;
        descriptor.stageFlags         = VK_SHADER_STAGE_ALL_GRAPHICS;
        descriptor.pImmutableSamplers = nullptr;
    }

    bool operator== (const InputBinding& other) const
    {
        return binding == other.binding;
    }
};

struct OutputBinding {
    uint32_t                binding;
    VkAttachmentDescription attachmentDescription;
    VkAttachmentReference   attachmentReference;

    OutputBinding (uint32_t binding)
        : binding (binding)
        , attachmentDescription ({})
        , attachmentReference ({})
    {
        attachmentDescription.format         = VK_FORMAT_R8G8B8A8_SRGB;
        attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_GENERAL; // TODO
        attachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_GENERAL; // TODO

        attachmentReference.attachment = binding;
        attachmentReference.layout     = VK_IMAGE_LAYOUT_GENERAL; // TODO
    }

    bool operator== (const InputBinding& other) const
    {
        return binding == other.binding;
    }
};


class Resource : public Noncopyable {
public:
    USING_PTR_ABSTRACT (Resource);

    virtual ~Resource () {}

    virtual void WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const = 0;
};

struct ImageResource final : public Resource {
    AllocatedImage image;
    ImageView::U   imageView;
    Sampler::U     sampler;
    Semaphore      semaphore;

    USING_PTR (ImageResource);

    ImageResource (const GraphInfo& graphInfo, const Device& device, VkQueue queue, VkCommandPool commandPool)
        : image (device, Image::Create (device, graphInfo.width, graphInfo.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT), DeviceMemory::GPU)
        , imageView (ImageView::Create (device, *image.image, image.image->GetFormat ()))
        , sampler (Sampler::Create (device))
        , semaphore (device)
    {
        SingleTimeCommand commandBuffer (device, commandPool, queue);
        image.image->CmdTransitionImageLayout (commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    }

    virtual ~ImageResource () {}

    virtual void WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const override
    {
        descriptorSet.WriteOneImageInfo (
            binding,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            {*sampler, *imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    }
};


struct ResourceVisitor final {
public:
    static void Visit (Resource& res, const std::function<void (ImageResource&)>& imageResourceTypeCallback)
    {
        ImageResource* imageType = dynamic_cast<ImageResource*> (&res);
        if (imageType != nullptr) {
            imageResourceTypeCallback (*imageType);
        } else {
            ERROR (true);
        }
    }
};


struct Operation : public Noncopyable {
    USING_PTR_ABSTRACT (Operation);

    std::vector<Resource::Ref> inputs;
    std::vector<Resource::Ref> outputs;

    std::vector<InputBinding>  inputBindings;
    std::vector<OutputBinding> outputBindings;

    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkSemaphore> signalSemaphores;

    CommandBuffer::U commandBuffer;
    VkCommandBuffer  commandBufferHandle;

    virtual ~Operation () {}

    virtual void Compile ()                             = 0;
    virtual void Record (VkCommandBuffer commandBuffer) = 0;

    void AddInput (uint32_t binding, const Resource::Ref& res)
    {
        ASSERT (std::find (inputBindings.begin (), inputBindings.end (), binding) == inputBindings.end ());

        inputs.push_back (res);
        inputBindings.push_back (binding);


        auto onImage = [&] (ImageResource& res) {
            waitSemaphores.push_back (res.semaphore);
        };

        ResourceVisitor::Visit (res.get (), onImage);
    }

    void AddOutput (uint32_t binding, const Resource::Ref& res)
    {
        ASSERT (std::find (outputBindings.begin (), outputBindings.end (), binding) == outputBindings.end ());

        outputs.push_back (res);
        outputBindings.push_back (binding);

        ResourceVisitor::Visit (res.get (), [&] (ImageResource& res) {
            signalSemaphores.push_back (res.semaphore);
        });
    }


    std::vector<VkAttachmentDescription> GetAttachmentDescriptions () const
    {
        std::vector<VkAttachmentDescription> result;
        for (const auto& t : outputBindings) {
            result.push_back (t.attachmentDescription);
        }
        return result;
    }

    std::vector<VkAttachmentReference> GetAttachmentReferences () const
    {
        std::vector<VkAttachmentReference> result;
        for (const auto& t : outputBindings) {
            result.push_back (t.attachmentReference);
        }
        return result;
    }

    std::vector<VkImageView> GetOutputImageViews () const
    {
        std::vector<VkImageView> result;

        auto onImage = [&] (ImageResource& res) {
            result.push_back (*res.imageView);
        };

        for (const auto& o : outputs) {
            ResourceVisitor::Visit (o, onImage);
        }

        return result;
    }

    VkSubmitInfo GetSubmitInfo () const
    {
        VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo result         = {};
        result.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        result.waitSemaphoreCount   = waitSemaphores.size ();
        result.pWaitSemaphores      = waitSemaphores.data ();
        result.pWaitDstStageMask    = &stage;
        result.commandBufferCount   = 1;
        result.pCommandBuffers      = &commandBufferHandle;
        result.signalSemaphoreCount = signalSemaphores.size ();
        result.pSignalSemaphores    = signalSemaphores.data ();
        return result;
    }
};


struct PresentOperation final : public Operation {
};


struct RenderOperation final : public Operation {
    USING_PTR (RenderOperation);

    const VkDevice device;

    ShaderPipeline         pipeline;
    Framebuffer::U         framebuffer;
    DescriptorPool::U      descriptorPool;
    DescriptorSet::U       descriptorSet;
    DescriptorSetLayout::U descriptorSetLayout;
    const GraphInfo        graphInfo;

    RenderOperation (const GraphInfo& graphInfo, VkDevice device, VkCommandPool commandPool, const std::vector<std::filesystem::path>& shaders)
        : graphInfo (graphInfo)
        , device (device)
    {
        ASSERT (!shaders.empty ());

        commandBuffer       = CommandBuffer::Create (device, commandPool);
        commandBufferHandle = *commandBuffer;
        pipeline.AddShaders (device, shaders);
    }

    virtual ~RenderOperation () {}

    virtual void Compile () override
    {
        std::vector<VkDescriptorSetLayoutBinding> layout;
        for (auto& inputBinding : inputBindings) {
            layout.push_back (inputBinding.descriptor);
        }

        descriptorSetLayout = DescriptorSetLayout::Create (device, layout);

        if (!inputBindings.empty ()) {
            descriptorPool = DescriptorPool::Create (device, 0, inputBindings.size (), 1);
            descriptorSet  = DescriptorSet::Create (device, *descriptorPool, *descriptorSetLayout);

            for (uint32_t i = 0; i < inputs.size (); ++i) {
                Resource& r = inputs[i];
                r.WriteToDescriptorSet (*descriptorSet, inputBindings[i].binding);
            }
        }

        pipeline.Compile (device, graphInfo.width, graphInfo.height, *descriptorSetLayout, GetAttachmentReferences (), GetAttachmentDescriptions ());

        framebuffer = Framebuffer::Create (device, *pipeline.renderPass, GetOutputImageViews (), graphInfo.width, graphInfo.height);
    }

    virtual void Record (VkCommandBuffer commandBuffer) override
    {
        VkClearValue              clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        std::vector<VkClearValue> clearValues (outputs.size (), clearColor);

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass            = *pipeline.renderPass;
        renderPassBeginInfo.framebuffer           = *framebuffer;
        renderPassBeginInfo.renderArea.offset     = {0, 0};
        renderPassBeginInfo.renderArea.extent     = {graphInfo.width, graphInfo.height};
        renderPassBeginInfo.clearValueCount       = clearValues.size ();
        renderPassBeginInfo.pClearValues          = clearValues.data ();

        vkCmdBeginRenderPass (commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline (commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline.pipeline);

        if (descriptorSet) {
            VkDescriptorSet dsHandle = *descriptorSet;

            vkCmdBindDescriptorSets (commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline.pipelineLayout, 0,
                                     1, &dsHandle,
                                     0, nullptr);
        }
        vkCmdDraw (commandBuffer, 3, 1, 0, 0);
        vkCmdEndRenderPass (commandBuffer);
    }
};


template<typename T>
class UniquePtrReferenceWrapper final {
private:
    T* ptr;

public:
    UniquePtrReferenceWrapper (const std::unique_ptr<T>& ptr)
        : ptr (ptr.get ())
    {
        ASSERT (ptr == nullptr);
    }

    UniquePtrReferenceWrapper (std::nullptr_t) = delete;

    ~UniquePtrReferenceWrapper () {}

    operator T& () const { return *ptr; }
    T& operator-> () const { return *ptr; }
    T& operator* () const { return *ptr; }
};


struct OperationDependency {
    // "after" must happen before "before"

    Operation::Ref before;
    Operation::Ref after;

    // "before" signals the semaphore
    // "after" waits on the semaphore
    Semaphore::U semaphore;
};


class Graph final : public Noncopyable {
public:
    USING_PTR (Graph);

    VkDevice         device;
    VkCommandPool    commandPool;
    CommandBuffer::U cmdBuf;

    std::vector<Resource::U>  resources;
    std::vector<Operation::U> operations;

    // TODO use these
    uint32_t width;
    uint32_t height;

    Graph (VkDevice device, VkCommandPool commandPool, uint32_t width, uint32_t height)
        : device (device)
        , commandPool (commandPool)
        , width (width)
        , height (height)
    {
    }

    GraphInfo GetGraphInfo ()
    {
        GraphInfo g;
        g.width  = width;
        g.height = height;
        return g;
    }

    Resource::Ref CreateResource (Resource::U&& resource)
    {
        resources.push_back (std::move (resource));
        return *resources[resources.size () - 1];
    }

    Operation::Ref CreateOperation (Operation::U&& resource)
    {
        operations.push_back (std::move (resource));
        return *operations[operations.size () - 1];
    }

    void Compile ()
    {
        for (auto& op : operations) {
            op->Compile ();
        }

        cmdBuf = CommandBuffer::Create (device, commandPool);
        cmdBuf->Begin ();
        for (auto& op : operations) {
            op->Record (*cmdBuf);

            if (&op != &operations.back ()) {
                VkMemoryBarrier memoryBarrier = {};
                memoryBarrier.srcAccessMask   = VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier (
                    *cmdBuf,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // srcStageMask
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,    // dstStageMask
                    0,
                    1, &memoryBarrier, // memory barriers
                    0, nullptr,        // buffer memory barriers
                    0, nullptr         // image barriers
                );
            }
        }
        cmdBuf->End ();
    }

    void Submit (VkQueue queue)
    {
        VkCommandBuffer cmdHdl = *cmdBuf;

        VkSubmitInfo result         = {};
        result.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        result.waitSemaphoreCount   = 0;
        result.pWaitSemaphores      = nullptr;
        result.pWaitDstStageMask    = 0;
        result.commandBufferCount   = 1;
        result.pCommandBuffers      = &cmdHdl;
        result.signalSemaphoreCount = 0;
        result.pSignalSemaphores    = nullptr;
        vkQueueSubmit (queue, 1, &result, nullptr);

        //std::vector<VkSubmitInfo> submitInfos;
        //
        //for (auto& a : operations) {
        //    submitInfos.push_back (a->GetSubmitInfo ());
        //}
        //vkQueueSubmit (queue, submitInfos.size (), submitInfos.data (), VK_NULL_HANDLE);
        //vkQueueWaitIdle (queue);
    }
}; // namespace RenderGraph

} // namespace RenderGraph

#endif