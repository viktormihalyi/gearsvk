#ifndef OPERATION_HPP
#define OPERATION_HPP

#include "Shaderpipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "Resource.hpp"

namespace RenderGraph {

struct InputBinding {
    const uint32_t               binding;
    VkDescriptorSetLayoutBinding descriptor;

    InputBinding (uint32_t binding);

    bool operator== (const InputBinding& other) const { return binding == other.binding; }
};


struct OutputBinding {
    uint32_t                binding;
    VkAttachmentDescription attachmentDescription;
    VkAttachmentReference   attachmentReference;

    OutputBinding (uint32_t binding);

    bool operator== (const InputBinding& other) const { return binding == other.binding; }
};


struct Operation : public Noncopyable {
    USING_PTR_ABSTRACT (Operation);

    std::vector<Resource::Ref> inputs;
    std::vector<Resource::Ref> outputs;

    std::vector<InputBinding>  inputBindings;
    std::vector<OutputBinding> outputBindings;

    virtual ~Operation () {}

    virtual void Compile ()                             = 0;
    virtual void Record (VkCommandBuffer commandBuffer) = 0;

    void AddInput (uint32_t binding, const Resource::Ref& res);
    void AddOutput (uint32_t binding, const Resource::Ref& res);


    std::vector<VkAttachmentDescription> GetAttachmentDescriptions () const;
    std::vector<VkAttachmentReference>   GetAttachmentReferences () const;
    std::vector<VkImageView>             GetOutputImageViews () const;
};


struct PresentOperation final : public Operation {
};

//
//struct LambdaOperation final : public Operation {
//public:
//    ShaderPipeline         pipeline;
//    Framebuffer::U         framebuffer;
//    DescriptorPool::U      descriptorPool;
//    DescriptorSet::U       descriptorSet;
//    DescriptorSetLayout::U descriptorSetLayout;
//    const GraphInfo        graphInfo;
//
//    std::function<void ()>                compileFunc;
//    std::function<void (VkCommandBuffer)> recordFunc;
//
//    LambdaOperation (const GraphInfo& graphInfo, VkDevice device, VkCommandPool commandPool, const std::vector<std::filesystem::path>& shaders,
//                     const std::function<void ()>&               compileFunc,
//                     const std::function<void (VkCommandBuffer)> recordFunc);
//
//    virtual void Compile () { compileFunc (); }
//    virtual void Record (VkCommandBuffer commandBuffer) { recordFunc (commandBuffer); }
//};

struct RenderOperation final : public Operation {
    USING_PTR (RenderOperation);

    const VkDevice device;

    ShaderPipeline::U      pipeline;
    Framebuffer::U         framebuffer;
    DescriptorPool::U      descriptorPool;
    DescriptorSet::U       descriptorSet;
    DescriptorSetLayout::U descriptorSetLayout;
    const GraphInfo        graphInfo;
    const uint32_t         vertexCount;

    RenderOperation (const GraphInfo& graphInfo, VkDevice device, VkCommandPool commandPool, uint32_t vertexCount, const std::vector<std::filesystem::path>& shaders);
    RenderOperation (const GraphInfo& graphInfo, VkDevice device, VkCommandPool commandPool, uint32_t vertexCount, ShaderPipeline::U&& shaderPipiline);
    
    virtual ~RenderOperation () {}

    virtual void Compile () override;

    virtual void Record (VkCommandBuffer commandBuffer) override;
};

} // namespace RenderGraph

#endif