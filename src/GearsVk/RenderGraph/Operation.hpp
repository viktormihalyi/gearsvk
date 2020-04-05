#ifndef OPERATION_HPP
#define OPERATION_HPP

#include "Shaderpipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "Connections.hpp"
#include "DrawRecordable.hpp"
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

struct RenderOperation final : public Operation {
    USING_PTR (RenderOperation);

    DescriptorPool::U      descriptorPool;
    DescriptorSetLayout::U descriptorSetLayout;
    const GraphSettings    graphSettings;

    std::vector<Framebuffer::U>   framebuffers;
    std::vector<DescriptorSet::U> descriptorSets;

    ShaderPipeline::P pipeline;
    DrawRecordable::P drawRecordable;

    RenderOperation (const GraphSettings& graphSettings, const DrawRecordable::P& drawRecordable, const ShaderPipeline::P& shaderPipiline);

    virtual ~RenderOperation () {}
    virtual void Compile () override;
    virtual void Record (uint32_t imageIndex, VkCommandBuffer commandBuffer) override;
};


} // namespace RenderGraph

#endif