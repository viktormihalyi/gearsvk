#ifndef OPERATION_HPP
#define OPERATION_HPP

#include "GearsVkAPI.hpp"

#include "ShaderPipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "Connections.hpp"
#include "DrawRecordable.hpp"
#include "Node.hpp"
#include "Resource.hpp"

#include <vector>

namespace RG {

USING_PTR (Operation);
struct GEARSVK_API Operation : public Node {
    USING_CREATE (Operation);

    std::vector<InputBindingU> inputBindings;
    std::vector<OutputBinding> outputBindings;

    virtual ~Operation () = default;

    virtual void Compile (const GraphSettings&, uint32_t width, uint32_t height) = 0;
    virtual void Record (uint32_t frameIndex, CommandBuffer& commandBuffer)      = 0;
    virtual bool IsActive ()                                                     = 0;

    void AddInput (InputBindingU&& inputBinding);
    void AddOutput (uint32_t binding, const ImageResourceRef& res);

    // when record called, input images will be in GetImageLayoutAtStartForInputs (),Resource&
    // output images will be in GetImageLayoutAtStartForOutputs () Resource&layouts.
    // recorded commands will put the input images in GetImageLayoutAtEndForInputs (),Resource&
    // output images in GetImageLayoutAtEndForOutputs () Resource&layouts.
    virtual VkImageLayout GetImageLayoutAtStartForInputs (Resource&)  = 0;
    virtual VkImageLayout GetImageLayoutAtEndForInputs (Resource&)    = 0;
    virtual VkImageLayout GetImageLayoutAtStartForOutputs (Resource&) = 0;
    virtual VkImageLayout GetImageLayoutAtEndForOutputs (Resource&)   = 0;

    std::vector<VkAttachmentDescription> GetAttachmentDescriptions () const;
    std::vector<VkAttachmentReference>   GetAttachmentReferences () const;
    std::vector<VkImageView>             GetOutputImageViews (uint32_t frameIndex) const;
};

USING_PTR (RenderOperation);
struct GEARSVK_API RenderOperation : public Operation {
    USING_CREATE (RenderOperation);

    struct CompileSettings {
        PureDrawRecordableP      drawRecordable;
        VertexAttributeProviderP vertexAttributeProvider;
        ShaderPipelineP          pipeline;
        VkPrimitiveTopology      topology;
    };

    struct CompileResult {
        uint32_t                    width;
        uint32_t                    height;
        DescriptorPoolU             descriptorPool;
        DescriptorSetLayoutU        descriptorSetLayout;
        std::vector<DescriptorSetU> descriptorSets;
        std::vector<FramebufferU>   framebuffers;

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


    RenderOperation (const DrawRecordableP& drawRecordable, const ShaderPipelineP& shaderPipiline, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    RenderOperation (const PureDrawRecordableP& drawRecordable, const VertexAttributeProviderP&, const ShaderPipelineP& shaderPipiline, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    virtual ~RenderOperation () = default;

    virtual VkImageLayout GetImageLayoutAtStartForInputs (Resource&) override;
    virtual VkImageLayout GetImageLayoutAtEndForInputs (Resource&) override;
    virtual VkImageLayout GetImageLayoutAtStartForOutputs (Resource&) override;
    virtual VkImageLayout GetImageLayoutAtEndForOutputs (Resource&) override;

    virtual void Compile (const GraphSettings&, uint32_t width, uint32_t height) override;
    virtual void Record (uint32_t imageIndex, CommandBuffer& commandBuffer) override;
    virtual bool IsActive () override { return true; }
};


USING_PTR (TransferOperation);
class GEARSVK_API TransferOperation : public Operation {
    USING_CREATE (TransferOperation);

public:
    TransferOperation ();

    // overriding Operation
    virtual void Compile (const GraphSettings&, uint32_t width, uint32_t height) override;
    virtual void Record (uint32_t imageIndex, CommandBuffer& commandBuffer) override;
    virtual bool IsActive () override { return true; }

    virtual VkImageLayout GetImageLayoutAtStartForInputs (Resource&) override { return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL; }
    virtual VkImageLayout GetImageLayoutAtEndForInputs (Resource&) override { return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL; }
    virtual VkImageLayout GetImageLayoutAtStartForOutputs (Resource&) override { return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; }
    virtual VkImageLayout GetImageLayoutAtEndForOutputs (Resource&) override { return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; }
};


} // namespace RG

#endif