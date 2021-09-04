#ifndef OPERATION_HPP
#define OPERATION_HPP

#include "RenderGraph/RenderGraphAPI.hpp"

#include "RenderGraph/Node.hpp"
#include "RenderGraph/ShaderReflectionToDescriptor.hpp"
#include "RenderGraph/ShaderReflectionToAttachment.hpp"

#include "VulkanWrapper/ShaderModule.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <optional>
#include <filesystem>
#include <vector>
#include <memory>


namespace GVK {
class DeviceExtra;
class DescriptorPool;
class DescriptorSet;
class DescriptorSetLayout;
class Framebuffer;
class ImageView2D;
class CommandBuffer;
} // namespace GVK

namespace RG {
class ShaderPipeline;
class Resource;
class GraphSettings;
class ConnectionSet;
class DrawRecordable;
class DrawRecordableInfo;
} // namespace RG

namespace RG {

class GVK_RENDERER_API Operation : public Node {
public:
    virtual ~Operation () override = default;

    virtual void Compile (const GraphSettings&, uint32_t width, uint32_t height)                                        = 0;
    virtual void Record (const ConnectionSet& connectionSet, uint32_t resourceIndex, GVK::CommandBuffer& commandBuffer) = 0;
    virtual bool IsActive ()                                                                                            = 0;

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

    class GVK_RENDERER_API Builder {
    private:
        VkDevice                           device;
        std::unique_ptr<DrawRecordable>    drawRecordable;
        std::unique_ptr<ShaderPipeline>    shaderPipiline;
        std::optional<VkPrimitiveTopology> topology;
        std::optional<glm::vec4>           clearColor;
        std::optional<bool>                blendEnabled;
        std::optional<std::string>         name;

    public:
        Builder (VkDevice device);

        Builder& SetPrimitiveTopology (VkPrimitiveTopology value);
        Builder& SetVertexShader (const std::string& value);
        Builder& SetFragmentShader (const std::string& value);
        Builder& SetVertexShader (const std::filesystem::path& value);
        Builder& SetFragmentShader (const std::filesystem::path& value);
        Builder& SetVertices (std::unique_ptr<DrawRecordable>&& value);
        Builder& SetBlendEnabled (bool value = true);
        Builder& SetClearColor (const glm::vec4& value);
        Builder& SetName (const std::string& value);

        std::shared_ptr<RenderOperation> Build ();

    private:
        void EnsurePipelineCreated ();
    };

    struct GVK_RENDERER_API CompileSettings {
        std::unique_ptr<DrawRecordable> drawRecordable;
        std::unique_ptr<ShaderPipeline> pipeline;
        VkPrimitiveTopology             topology;

        std::optional<glm::vec4> clearColor;   // (0, 0, 0, 1) by default
        std::optional<bool>      blendEnabled; // true by default

        std::unique_ptr<RG::FromShaderReflection::DescriptorWriteInfoTable> descriptorWriteProvider;
        std::unique_ptr<RG::FromShaderReflection::AttachmentDataTable>      attachmentProvider;
    };

    struct GVK_RENDERER_API CompileResult {
        uint32_t                                         width;
        uint32_t                                         height;
        std::unique_ptr<GVK::DescriptorPool>             descriptorPool;
        std::unique_ptr<GVK::DescriptorSetLayout>        descriptorSetLayout;
        std::vector<std::unique_ptr<GVK::DescriptorSet>> descriptorSets;
        std::vector<std::unique_ptr<GVK::Framebuffer>>   framebuffers;

        void Clear ();
    };

    CompileSettings compileSettings;
    CompileResult   compileResult;

    RenderOperation (std::unique_ptr<DrawRecordable>&& drawRecordable, std::unique_ptr<ShaderPipeline>&& shaderPipiline, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    virtual ~RenderOperation () override = default;

    virtual void Compile (const GraphSettings&, uint32_t width, uint32_t height) override;
    virtual void Record (const ConnectionSet& connectionSet, uint32_t imageIndex, GVK::CommandBuffer& commandBuffer) override;
    virtual bool IsActive () override { return true; }

    const std::unique_ptr<ShaderPipeline>& GetShaderPipeline () const { return compileSettings.pipeline; }

private:
    virtual VkImageLayout GetImageLayoutAtStartForInputs (Resource&) override;
    virtual VkImageLayout GetImageLayoutAtEndForInputs (Resource&) override;
    virtual VkImageLayout GetImageLayoutAtStartForOutputs (Resource&) override;
    virtual VkImageLayout GetImageLayoutAtEndForOutputs (Resource&) override;

private:
    void CompileDescriptors (const GraphSettings&);
};


#if 0
class GVK_RENDERER_API TransferOperation : public Operation {
public:
    TransferOperation ();

    // overriding Operation
    virtual void Compile (const GraphSettings&, uint32_t width, uint32_t height) override;
    virtual void Record (const ConnectionSet& connectionSet, uint32_t imageIndex, GVK::CommandBuffer& commandBuffer) override;
    virtual bool IsActive () override { return true; }

    virtual VkImageLayout GetImageLayoutAtStartForInputs (Resource&) override { return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL; }
    virtual VkImageLayout GetImageLayoutAtEndForInputs (Resource&) override { return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL; }
    virtual VkImageLayout GetImageLayoutAtStartForOutputs (Resource&) override { return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; }
    virtual VkImageLayout GetImageLayoutAtEndForOutputs (Resource&) override { return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; }
};
#endif

} // namespace RG

#endif