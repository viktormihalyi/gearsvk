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
    std::vector<InputBindingU> inputBindings;
    std::vector<OutputBinding> outputBindings;

    virtual ~Operation () = default;

    virtual void Compile (const GraphSettings&)                              = 0;
    virtual void Record (uint32_t frameIndex, VkCommandBuffer commandBuffer) = 0;

    void AddOutput (uint32_t binding, const ImageResourceRef& res);

    std::vector<VkAttachmentDescription> GetAttachmentDescriptions () const;
    std::vector<VkAttachmentReference>   GetAttachmentReferences () const;
    std::vector<VkImageView>             GetOutputImageViews (uint32_t frameIndex) const;
};

USING_PTR (RenderOperation);

struct GEARSVK_API RenderOperation : public Operation {
    USING_CREATE (RenderOperation);

    struct CompileSettings {
        // TODO add width and height here, remove them from GraphSettings, add them to WritableImageResouce as well
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

    virtual void Compile (const GraphSettings&) override;
    virtual void Record (uint32_t imageIndex, VkCommandBuffer commandBuffer) override;
};

} // namespace RG

#endif