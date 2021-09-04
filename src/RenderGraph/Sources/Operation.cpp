#include "Operation.hpp"
#include "ShaderReflectionToDescriptor.hpp"

#include "GraphSettings.hpp"
#include "Resource.hpp"
#include "ShaderPipeline.hpp"
#include "DrawRecordable.hpp"

#include "VulkanWrapper/CommandBuffer.hpp"
#include "VulkanWrapper/Commands.hpp"
#include "VulkanWrapper/DescriptorSet.hpp"
#include "VulkanWrapper/DescriptorSetLayout.hpp"
#include "VulkanWrapper/ShaderModule.hpp"
#include "VulkanWrapper/DescriptorSetLayout.hpp"
#include "VulkanWrapper/Event.hpp"
#include "VulkanWrapper/Framebuffer.hpp"
#include "VulkanWrapper/Image.hpp"
#include "VulkanWrapper/ImageView.hpp"
#include "VulkanWrapper/Pipeline.hpp"
#include "VulkanWrapper/PipelineLayout.hpp"
#include "VulkanWrapper/RenderPass.hpp"

#include "spdlog/spdlog.h"

#include <memory>


namespace RG {

RenderOperation::Builder::Builder (VkDevice device)
    : device (device)
{
}


RenderOperation::Builder& RenderOperation::Builder::SetPrimitiveTopology (VkPrimitiveTopology value)
{
    topology = value;
    return *this;
}


RenderOperation::Builder& RenderOperation::Builder::SetVertexShader (const std::string& value)
{
    EnsurePipelineCreated ();
    shaderPipiline->SetVertexShaderFromString (value);
    return *this;
}


RenderOperation::Builder& RenderOperation::Builder::SetFragmentShader (const std::string& value)
{
    EnsurePipelineCreated ();
    shaderPipiline->SetFragmentShaderFromString (value);
    return *this;
}


RenderOperation::Builder& RenderOperation::Builder::SetVertexShader (const std::filesystem::path& value)
{
    EnsurePipelineCreated ();
    shaderPipiline->SetShaderFromSourceFile (value);
    return *this;
}


RenderOperation::Builder& RenderOperation::Builder::SetFragmentShader (const std::filesystem::path& value)
{
    EnsurePipelineCreated ();
    shaderPipiline->SetShaderFromSourceFile (value);
    return *this;
}


RenderOperation::Builder& RenderOperation::Builder::SetVertices (std::unique_ptr<DrawRecordable>&& value)
{
    drawRecordable = std::move (value);
    return *this;
}


RenderOperation::Builder& RenderOperation::Builder::SetBlendEnabled (bool value)
{
    blendEnabled = value;
    return *this;
}


RenderOperation::Builder& RenderOperation::Builder::SetName (const std::string& value)
{
    name = value;
    return *this;
}


RenderOperation::Builder& RenderOperation::Builder::SetClearColor (const glm::vec4& value)
{
    clearColor = value;
    return *this;
}


std::shared_ptr<RenderOperation> RenderOperation::Builder::Build ()
{
    GVK_ASSERT (drawRecordable != nullptr);
    GVK_ASSERT (shaderPipiline != nullptr);

    std::shared_ptr<RenderOperation> op = std::make_shared<RenderOperation> (std::move (drawRecordable), std::move (shaderPipiline), *topology);

    drawRecordable = nullptr;
    shaderPipiline     = nullptr;

    if (name.has_value ())
        op->SetName (*name);

    return op;
}


void RenderOperation::Builder::EnsurePipelineCreated ()
{
    if (shaderPipiline == nullptr) {
        shaderPipiline = std::make_unique<ShaderPipeline> (device);
    }
}


RenderOperation::RenderOperation (std::unique_ptr<DrawRecordable>&& drawRecordable, std::unique_ptr<ShaderPipeline>&& shaderPipeline, VkPrimitiveTopology topology)
    : compileSettings ({ std::move (drawRecordable), std::move (shaderPipeline), topology })
{
    compileSettings.descriptorWriteProvider = std::make_unique<RG::FromShaderReflection::DescriptorWriteInfoTable> ();
    compileSettings.attachmentProvider = std::make_unique<RG::FromShaderReflection::AttachmentDataTable> ();
}


void RenderOperation::CompileResult::Clear ()
{
    descriptorPool.reset ();
    descriptorSetLayout.reset ();
    descriptorSets.clear ();
    framebuffers.clear ();
}


namespace {

class DescriptorCounter : public RG::FromShaderReflection::IUpdateDescriptorSets {
public:
    std::vector<VkDescriptorPoolSize> poolSizes;

    uint32_t multiplier = 1;

    virtual void UpdateDescriptorSets (const std::vector<VkWriteDescriptorSet>& writes) override
    {
        for (const VkWriteDescriptorSet& write : writes) {
            VkDescriptorPoolSize& poolSize = GetPoolSizeByType (write.descriptorType);
            poolSize.descriptorCount += write.descriptorCount * multiplier;
        }
    }

private:
    VkDescriptorPoolSize& GetPoolSizeByType (VkDescriptorType type)
    {
        for (VkDescriptorPoolSize& poolSize : poolSizes) {
            if (poolSize.type == type) {
                return poolSize;
            }
        }

        poolSizes.push_back ({ type, 0 });
        return poolSizes.back ();
    }
};


class DescriptorWriter : public RG::FromShaderReflection::IUpdateDescriptorSets {
public:
    VkDevice device = VK_NULL_HANDLE;

    virtual void UpdateDescriptorSets (const std::vector<VkWriteDescriptorSet>& writes) override
    {
        vkUpdateDescriptorSets (device, writes.size (), writes.data (), 0, nullptr);
    }
};

} // namespace


void RenderOperation::CompileDescriptors (const GraphSettings& graphSettings)
{
    compileResult.descriptorSetLayout = GetShaderPipeline ()->CreateDescriptorSetLayout (graphSettings.GetDevice ());

    DescriptorCounter descriptorCounter;
    descriptorCounter.multiplier = graphSettings.framesInFlight;
    GetShaderPipeline ()->IterateShaders ([&] (const GVK::ShaderModule& shaderModule) {
        RG::FromShaderReflection::WriteDescriptors (shaderModule.GetReflection (), VK_NULL_HANDLE, 0, shaderModule.GetShaderKind (), *compileSettings.descriptorWriteProvider, descriptorCounter);
    });

    if (!descriptorCounter.poolSizes.empty ()) {
        compileResult.descriptorPool = std::make_unique<GVK::DescriptorPool> (graphSettings.GetDevice (), descriptorCounter.poolSizes, graphSettings.framesInFlight);

        DescriptorWriter descriptorWriter;
        descriptorWriter.device = graphSettings.GetDevice ();

        for (uint32_t resourceIndex = 0; resourceIndex < graphSettings.framesInFlight; ++resourceIndex) {
            std::unique_ptr<GVK::DescriptorSet> descriptorSet = std::make_unique<GVK::DescriptorSet> (graphSettings.GetDevice (), *compileResult.descriptorPool, *compileResult.descriptorSetLayout);

            GetShaderPipeline ()->IterateShaders ([&] (const GVK::ShaderModule& shaderModule) {
                RG::FromShaderReflection::WriteDescriptors (shaderModule.GetReflection (), *descriptorSet, resourceIndex, shaderModule.GetShaderKind (), *compileSettings.descriptorWriteProvider, descriptorWriter);
            });

            compileResult.descriptorSets.push_back (std::move (descriptorSet));
        }
    }
}


void RenderOperation::Compile (const GraphSettings& graphSettings, uint32_t width, uint32_t height)
{
    compileResult.Clear ();

    CompileDescriptors (graphSettings);

    std::vector<std::vector<VkImageView>> imageViews;
    for (uint32_t resourceIndex = 0; resourceIndex < graphSettings.framesInFlight; ++resourceIndex) {
        imageViews.push_back (RG::FromShaderReflection::GetImageViews (GetShaderPipeline ()->fragmentShader->GetReflection (), GVK::ShaderKind::Fragment, resourceIndex, *compileSettings.attachmentProvider));
    }

    const std::vector<VkAttachmentReference>   attachmentReferences = RG::FromShaderReflection::GetAttachmentReferences (GetShaderPipeline ()->fragmentShader->GetReflection (), GVK::ShaderKind::Fragment, *compileSettings.attachmentProvider);
    const std::vector<VkAttachmentDescription> attachmentDescriptions = RG::FromShaderReflection::GetAttachmentDescriptions (GetShaderPipeline ()->fragmentShader->GetReflection (), GVK::ShaderKind::Fragment, *compileSettings.attachmentProvider);

    if constexpr (IsDebugBuild) {
        GVK_ASSERT (attachmentReferences.size () == attachmentDescriptions.size ());
        for (uint32_t resourceIndex = 0; resourceIndex < graphSettings.framesInFlight; ++resourceIndex) {
            const auto outputImageView = imageViews[resourceIndex];
            GVK_ASSERT (attachmentReferences.size () == outputImageView.size ());
        }
    }

    ShaderPipeline::CompileSettings pipelineSettigns = { width,
                                                         height,
                                                         compileResult.descriptorSetLayout->operator VkDescriptorSetLayout (),
                                                         attachmentReferences,
                                                         attachmentDescriptions,
                                                         compileSettings.topology };

    pipelineSettigns.blendEnabled = compileSettings.blendEnabled;

    GetShaderPipeline ()->Compile (std::move (pipelineSettigns));

    for (uint32_t resourceIndex = 0; resourceIndex < graphSettings.framesInFlight; ++resourceIndex) {
        compileResult.framebuffers.push_back (std::make_unique<GVK::Framebuffer> (graphSettings.GetDevice (),
                                                                                  *GetShaderPipeline ()->compileResult.renderPass,
                                                                                  imageViews[resourceIndex],
                                                                                  width,
                                                                                  height));
    }

    compileResult.width  = width;
    compileResult.height = height;
}


void RenderOperation::Record (const ConnectionSet& connectionSet, uint32_t resourceIndex, GVK::CommandBuffer& commandBuffer)
{
    uint32_t outputCount = 0;

    IResourceVisitorFn outputCounter ([&] (ReadOnlyImageResource&) {},
                                      [&] (WritableImageResource& res) { outputCount += res.GetLayerCount (); },
                                      [&] (SwapchainImageResource& res) { outputCount += res.GetLayerCount (); },
                                      [&] (GPUBufferResource&) {},
                                      [&] (CPUBufferResource&) {});

    connectionSet.VisitOutputsOf (this, outputCounter);

    VkClearValue clearColor     = {};
    clearColor.color.float32[0] = 0.0f;
    clearColor.color.float32[1] = 0.0f;
    clearColor.color.float32[2] = 0.0f;
    clearColor.color.float32[3] = 1.0f;
    clearColor = compileSettings.clearColor.has_value ()
                                  ? VkClearValue {
                                        compileSettings.clearColor->x,
                                        compileSettings.clearColor->y,
                                        compileSettings.clearColor->z,
                                        compileSettings.clearColor->w
                                    }
                                  : VkClearValue { 0.0f, 0.0f, 0.0f, 1.0f };

    std::vector<VkClearValue> clearValues (outputCount, clearColor);

    GVK_ASSERT (GetShaderPipeline () != nullptr);

    commandBuffer.Record<GVK::CommandBeginRenderPass> (*GetShaderPipeline ()->compileResult.renderPass,
                                                       *compileResult.framebuffers[resourceIndex],
                                                       VkRect2D { { 0, 0 }, { compileResult.width, compileResult.height } },
                                                       clearValues,
                                                       VK_SUBPASS_CONTENTS_INLINE)
        .SetName ("Operation - Renderpass Begin");

    commandBuffer.Record<GVK::CommandBindPipeline> (VK_PIPELINE_BIND_POINT_GRAPHICS, *GetShaderPipeline ()->compileResult.pipeline).SetName ("Operation - Bind");

    if (!compileResult.descriptorSets.empty ()) {
        VkDescriptorSet dsHandle = *compileResult.descriptorSets[resourceIndex];

        commandBuffer.Record<GVK::CommandBindDescriptorSets> (
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            *GetShaderPipeline ()->compileResult.pipelineLayout,
            0,
            std::vector<VkDescriptorSet> { dsHandle },
            std::vector<uint32_t> {}).SetName ("Operation - DescriptionSet");
    }

    GVK_ASSERT (compileSettings.drawRecordable != nullptr);
    compileSettings.drawRecordable->Record (commandBuffer);
    
    commandBuffer.Record<GVK::CommandEndRenderPass> ().SetName ("Operation - Renderpass End");
}


VkImageLayout RenderOperation::GetImageLayoutAtStartForInputs (Resource&)
{
    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}


VkImageLayout RenderOperation::GetImageLayoutAtEndForInputs (Resource&)
{
    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}


VkImageLayout RenderOperation::GetImageLayoutAtStartForOutputs (Resource& res)
{
    if (auto sw = dynamic_cast<SwapchainImageResource*> (&res)) {
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    } else {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
}


VkImageLayout RenderOperation::GetImageLayoutAtEndForOutputs (Resource& res)
{
    if (auto sw = dynamic_cast<SwapchainImageResource*> (&res)) {
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    } else {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
}


#if 0
TransferOperation::TransferOperation ()
{
}


void TransferOperation::Compile (const GraphSettings&, uint32_t, uint32_t)
{
    // nothing to compile
}


void TransferOperation::Record (const ConnectionSet& connectionSet, uint32_t imageIndex, GVK::CommandBuffer& commandBuffer)
{
    std::vector<ImageResource*> inputs;
    std::vector<ImageResource*> outputs;

    IResourceVisitorFn inputGatherer ([&] (ReadOnlyImageResource& resource) { inputs.push_back (&resource); },
                                      [&] (WritableImageResource& resource) { inputs.push_back (&resource); },
                                      [&] (SwapchainImageResource& resource) { inputs.push_back (&resource); },
                                      [&] (GPUBufferResource&) {},
                                      [&] (CPUBufferResource&) {});

    IResourceVisitorFn outputGatherer ([&] (ReadOnlyImageResource& resource) { outputs.push_back (&resource); },
                                       [&] (WritableImageResource& resource) { outputs.push_back (&resource); },
                                       [&] (SwapchainImageResource& resource) { outputs.push_back (&resource); },
                                       [&] (GPUBufferResource&) {},
                                       [&] (CPUBufferResource&) {});

    connectionSet.VisitOutputsOf (this, outputGatherer);

    if (GVK_ERROR (inputs.size () != 1)) {
        throw std::runtime_error ("no");
    }

    if (GVK_ERROR (outputs.size () != 1)) {
        throw std::runtime_error ("no");
    }

    ImageResource* from = inputs[0];
    ImageResource* to   = outputs[0];

    if (GVK_ERROR (from == to)) {
        throw std::runtime_error ("no");
    }

    if (auto fromImg = dynamic_cast<ImageResource*> (from)) {
        if (auto toImg = dynamic_cast<ImageResource*> (to)) {
            GVK::Image* fromVkImage = fromImg->GetImages ()[0];
            GVK::Image* toVkImage   = toImg->GetImages ()[0];

            GVK_ASSERT (fromVkImage->GetWidth () == toVkImage->GetWidth ());
            GVK_ASSERT (fromVkImage->GetHeight () == toVkImage->GetHeight ());
            GVK_ASSERT (fromVkImage->GetDepth () == toVkImage->GetDepth ());

            const uint32_t layerIndex = 0;

            VkImageCopy imageCopyRegion                   = {};
            imageCopyRegion.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopyRegion.srcSubresource.layerCount     = 1;
            imageCopyRegion.srcSubresource.baseArrayLayer = layerIndex;
            imageCopyRegion.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopyRegion.dstSubresource.layerCount     = 1;
            imageCopyRegion.extent.width                  = fromVkImage->GetWidth ();
            imageCopyRegion.extent.height                 = fromVkImage->GetHeight ();
            imageCopyRegion.extent.depth                  = fromVkImage->GetDepth ();

            commandBuffer.Record<GVK::CommandCopyImage> (
                    *fromVkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    *toVkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    std::vector<VkImageCopy> { imageCopyRegion});
            return;
        }
    }

    GVK_ASSERT ("bad resources");
    throw std::runtime_error ("unknown resources to transfer");
}
#endif


} // namespace RG
