#ifndef OPERATION_HPP
#define OPERATION_HPP

#include "Shaderpipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "Connections.hpp"
#include "Resource.hpp"

namespace RenderGraph {

struct Operation : public Noncopyable {
    USING_PTR_ABSTRACT (Operation);

    std::vector<Resource::Ref> inputs;
    std::vector<Resource::Ref> outputs;

    std::vector<InputBinding>  inputBindings;
    std::vector<OutputBinding> outputBindings;

    virtual ~Operation () {}

    virtual void Compile ()                                                  = 0;
    virtual void Record (uint32_t frameIndex, VkCommandBuffer commandBuffer) = 0;

    void AddInput (uint32_t binding, const Resource::Ref& res);
    void AddOutput (uint32_t binding, const Resource::Ref& res);

    std::vector<VkAttachmentDescription> GetAttachmentDescriptions () const;
    std::vector<VkAttachmentReference>   GetAttachmentReferences () const;
    std::vector<VkImageView>             GetOutputImageViews (uint32_t frameIndex) const;
};


struct RenderOperationSettings {
    const uint32_t instanceCount;

    const uint32_t                                       vertexCount;
    const VkBuffer                                       vertexBuffer;
    const std::vector<VkVertexInputBindingDescription>   vertexInputBindings;
    const std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;

    const uint32_t indexCount;
    const VkBuffer indexBuffer;

    RenderOperationSettings (const uint32_t                                        instanceCount,
                             uint32_t                                              vertexCount,
                             VkBuffer                                              vertexBuffer          = VK_NULL_HANDLE,
                             const std::vector<VkVertexInputBindingDescription>&   vertexInputBindings   = {},
                             const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributes = {},
                             uint32_t                                              indexCount            = 0,
                             VkBuffer                                              indexBuffer           = VK_NULL_HANDLE)
        : instanceCount (instanceCount)
        , vertexCount (vertexCount)
        , vertexBuffer (vertexBuffer)
        , vertexInputBindings (vertexInputBindings)
        , vertexInputAttributes (vertexInputAttributes)
        , indexCount (indexCount)
        , indexBuffer (indexBuffer)
    {
    }

    RenderOperationSettings (const uint32_t                         instanceCount,
                             const UntypedTransferableVertexBuffer& vertexBuffer,
                             const TransferableIndexBuffer&         indexBuffer)
        : instanceCount (instanceCount)
        , vertexCount (vertexBuffer.data.size ())
        , vertexBuffer (vertexBuffer.buffer.GetBufferToBind ())
        , vertexInputBindings (vertexBuffer.info.bindings)
        , vertexInputAttributes (vertexBuffer.info.attributes)
        , indexCount (indexBuffer.data.size ())
        , indexBuffer (indexBuffer.buffer.GetBufferToBind ())
    {
    }
};

struct RenderOperation final : public Operation {
    USING_PTR (RenderOperation);

    ShaderPipeline::U      pipeline;
    DescriptorPool::U      descriptorPool;
    DescriptorSetLayout::U descriptorSetLayout;
    const GraphSettings    graphSettings;

    std::vector<Framebuffer::U>   framebuffers;
    std::vector<DescriptorSet::U> descriptorSets;

    const RenderOperationSettings settings;

    RenderOperation (const GraphSettings& graphSettings, const RenderOperationSettings& settings, const std::vector<std::filesystem::path>& shaders);
    RenderOperation (const GraphSettings& graphSettings, const RenderOperationSettings& settings, ShaderPipeline::U&& shaderPipiline);

    virtual ~RenderOperation () {}
    virtual void Compile () override;
    virtual void Record (uint32_t imageIndex, VkCommandBuffer commandBuffer) override;
};


} // namespace RenderGraph

#endif