#ifndef OPERATION_HPP
#define OPERATION_HPP

#include "GearsVkAPI.hpp"

#include "ShaderPipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "Connections.hpp"
#include "DrawRecordable.hpp"
#include "Resource.hpp"


namespace RG {

struct GEARSVK_API Operation : public Noncopyable {
    USING_PTR_ABSTRACT (Operation);

    std::vector<Resource::Ref> inputs;
    std::vector<Resource::Ref> outputs;

    std::vector<IInputBinding::U> newInputBindings;
    std::vector<OutputBinding>    outputBindings;

    virtual ~Operation () {}

    virtual void Compile (const GraphSettings&)                              = 0;
    virtual void Record (uint32_t frameIndex, VkCommandBuffer commandBuffer) = 0;

    void AddOutput (uint32_t binding, const ImageResource::Ref& res);

    std::vector<VkAttachmentDescription> GetAttachmentDescriptions () const;
    std::vector<VkAttachmentReference>   GetAttachmentReferences () const;
    std::vector<VkImageView>             GetOutputImageViews (uint32_t frameIndex) const;
};


struct GEARSVK_API RenderOperation : public Operation {
    USING_PTR (RenderOperation);

    struct CompileSettings {
        DrawRecordable::P drawRecordable;
        ShaderPipeline::P pipeline;
    };

    struct CompileResult {
        uint32_t                      width;
        uint32_t                      height;
        DescriptorPool::U             descriptorPool;
        DescriptorSetLayout::U        descriptorSetLayout;
        std::vector<DescriptorSet::U> descriptorSets;
        std::vector<Framebuffer::U>   framebuffers;

        void Clear ()
        {
            descriptorPool.reset ();
            descriptorSetLayout.reset ();
            descriptorSets.clear ();
            framebuffers.clear ();
        }
    };

    CompileSettings compileSettings;
    CompileResult   compileResult;


    RenderOperation (const DrawRecordable::P& drawRecordable, const ShaderPipeline::P& shaderPipiline);
    virtual ~RenderOperation () = default;

    virtual void Compile (const GraphSettings&) override;
    virtual void Record (uint32_t imageIndex, VkCommandBuffer commandBuffer) override;

    ShaderBlocks& operator () (ShaderModule::ShaderKind shaderKind)
    {
        return *(compileSettings.pipeline->GetShaderByKind (shaderKind).ubos);
    }

};

} // namespace RG

#endif