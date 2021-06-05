#ifndef OPERATION_HPP
#define OPERATION_HPP

#include "GearsVkAPI.hpp"

#include "Utils/Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "Connections.hpp"
#include "Node.hpp"

#include "glmlib.hpp"

#include <vector>


namespace GVK {

class DrawRecordable;
class DrawRecordableInfo;
class PureDrawRecordable;
class VertexAttributeProvider;

namespace RG {

class ShaderPipeline;

class Resource;
class GraphSettings;
class ConnectionSet;

class GVK_RENDERER_API Operation : public Node {
public:
    virtual ~Operation () = default;

    virtual void Compile (const GraphSettings&, uint32_t width, uint32_t height)                                = 0;
    virtual void Record (const ConnectionSet& connectionSet, uint32_t resourceIndex, CommandBuffer& commandBuffer) = 0;
    virtual bool IsActive ()                                                                                    = 0;

    // when record called, input images will be in GetImageLayoutAtStartForInputs ()
    // output images will be in GetImageLayoutAtStartForOutputs () layouts.
    // recorded commands will put the input images in GetImageLayoutAtEndForInputs ()
    // output images in GetImageLayoutAtEndForOutputs () layouts.
    virtual VkImageLayout GetImageLayoutAtStartForInputs (Resource&)  = 0;
    virtual VkImageLayout GetImageLayoutAtEndForInputs (Resource&)    = 0;
    virtual VkImageLayout GetImageLayoutAtStartForOutputs (Resource&) = 0;
    virtual VkImageLayout GetImageLayoutAtEndForOutputs (Resource&)   = 0;
};

class GVK_RENDERER_API RenderOperation : public Operation {
public:
    struct CompileSettings {
        std::unique_ptr<PureDrawRecordable>      drawRecordable;
        std::unique_ptr<VertexAttributeProvider> vertexAttributeProvider;
        std::unique_ptr<ShaderPipeline>          pipeline;
        VkPrimitiveTopology                      topology;

        std::optional<glm::vec4> clearColor;   // (0, 0, 0, 1) by default
        std::optional<bool>      blendEnabled; // true by default
    };

    struct CompileResult {
        uint32_t                                    width;
        uint32_t                                    height;
        std::unique_ptr<DescriptorPool>             descriptorPool;
        std::unique_ptr<DescriptorSetLayout>        descriptorSetLayout;
        std::vector<std::unique_ptr<DescriptorSet>> descriptorSets;
        std::vector<std::unique_ptr<Framebuffer>>   framebuffers;

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


    RenderOperation (std::unique_ptr<DrawRecordable>&& drawRecordable, std::unique_ptr<ShaderPipeline>&& shaderPipiline, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    RenderOperation (std::unique_ptr<PureDrawRecordable>&& drawRecordable, std::unique_ptr<VertexAttributeProvider>&&, std::unique_ptr<ShaderPipeline>&& shaderPipiline, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    virtual ~RenderOperation () = default;

    virtual VkImageLayout GetImageLayoutAtStartForInputs (Resource&) override;
    virtual VkImageLayout GetImageLayoutAtEndForInputs (Resource&) override;
    virtual VkImageLayout GetImageLayoutAtStartForOutputs (Resource&) override;
    virtual VkImageLayout GetImageLayoutAtEndForOutputs (Resource&) override;

    virtual void Compile (const GraphSettings&, uint32_t width, uint32_t height) override;
    virtual void Record (const ConnectionSet& connectionSet, uint32_t imageIndex, CommandBuffer& commandBuffer) override;
    virtual bool IsActive () override { return true; }

private:
    // helper functions

    std::vector<VkImageView>             GetOutputImageViews (const ConnectionSet& conncetionSet, uint32_t resourceIndex) const;
    std::vector<VkAttachmentDescription> GetAttachmentDescriptions (const ConnectionSet& conncetionSet) const;
    std::vector<VkAttachmentReference>   GetAttachmentReferences (const ConnectionSet& conncetionSet) const;
};


class GVK_RENDERER_API TransferOperation : public Operation {
public:
    TransferOperation ();

    // overriding Operation
    virtual void Compile (const GraphSettings&, uint32_t width, uint32_t height) override;
    virtual void Record (const ConnectionSet& connectionSet, uint32_t imageIndex, CommandBuffer& commandBuffer) override;
    virtual bool IsActive () override { return true; }

    virtual VkImageLayout GetImageLayoutAtStartForInputs (Resource&) override { return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL; }
    virtual VkImageLayout GetImageLayoutAtEndForInputs (Resource&) override { return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL; }
    virtual VkImageLayout GetImageLayoutAtStartForOutputs (Resource&) override { return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; }
    virtual VkImageLayout GetImageLayoutAtEndForOutputs (Resource&) override { return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; }
};


} // namespace RG

} // namespace GVK


#endif