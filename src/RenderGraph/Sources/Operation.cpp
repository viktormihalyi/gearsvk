#include "Operation.hpp"

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


RenderOperation::Builder& RenderOperation::Builder::SetVertices (std::unique_ptr<PureDrawRecordable>&& value)
{
    pureDrawRecordable = std::move (value);
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
    GVK_ASSERT (pureDrawRecordable != nullptr);
    GVK_ASSERT (shaderPipiline != nullptr);

    std::shared_ptr<RenderOperation> op = std::make_shared<RenderOperation> (std::move (pureDrawRecordable), std::move (shaderPipiline), *topology);

    pureDrawRecordable = nullptr;
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


std::vector<VkAttachmentDescription> RenderOperation::GetAttachmentDescriptions (const ConnectionSet& connectionSet) const
{
    std::vector<VkAttachmentDescription> result;

    connectionSet.ProcessOutputBindingsOf (this, [&] (const IConnectionBinding& binding) {
        for (auto& a : binding.GetAttachmentDescriptions ()) {
            result.push_back (a);
        }
    });

    return result;
}


std::vector<VkAttachmentReference> RenderOperation::GetAttachmentReferences (const ConnectionSet& connectionSet) const
{
    std::vector<VkAttachmentReference> result;

    connectionSet.ProcessOutputBindingsOf (this, [&] (const IConnectionBinding& binding) {
        for (auto& a : binding.GetAttachmentReferences ()) {
            result.push_back (a);
        }
    });

    return result;
}


std::vector<VkImageView> RenderOperation::GetOutputImageViews (const ConnectionSet& conncetionSet, uint32_t resourceIndex) const
{
    std::vector<VkImageView> result;

    IResourceVisitorFn outputGatherer ([] (ReadOnlyImageResource&) {},
                                       [&] (WritableImageResource& res) {
        for (auto& imgView : res.images[resourceIndex]->imageViews) {
            result.push_back (*imgView);
        } },
                                       [&] (SwapchainImageResource& res) {
                                           result.push_back (*res.imageViews[resourceIndex]);
                                       },
                                       [] (GPUBufferResource&) {},
                                       [] (CPUBufferResource&) {});


    conncetionSet.VisitOutputsOf (this, outputGatherer);


    return result;
}


std::vector<GVK::ImageView2D> RenderOperation::CreateOutputImageViews (const GVK::DeviceExtra& device, const ConnectionSet& conncetionSet, uint32_t resourceIndex) const
{
    std::vector<GVK::ImageView2D> result;

    IResourceVisitorFn outputGatherer ([] (ReadOnlyImageResource&) {},
                                       [&] (WritableImageResource& res) {
        for (auto&& imgView : res.images[resourceIndex]->CreateImageViews (device)) {
            result.push_back (std::move (imgView));
        }},
                                       [&] (SwapchainImageResource& res) {
        for (auto&& imgView : res.CreateImageViews (device)) {
            result.push_back (std::move (imgView));
        }},
                                       [] (GPUBufferResource&) {},
                                       [] (CPUBufferResource&) {});


    conncetionSet.VisitOutputsOf (this, outputGatherer);


    return result;
}


RenderOperation::RenderOperation (std::unique_ptr<PureDrawRecordable>&& drawRecordable, std::unique_ptr<ShaderPipeline>&& shaderPipeline, VkPrimitiveTopology topology)
    : compileSettings ({ std::move (drawRecordable), std::move (shaderPipeline), topology })
{
}


void RenderOperation::CompileResult::Clear ()
{
    descriptorPool.reset ();
    descriptorSetLayout.reset ();
    descriptorSets.clear ();
    framebuffers.clear ();
}


void RenderOperation::Compile (const GraphSettings& graphSettings, uint32_t width, uint32_t height)
{
    compileResult.Clear ();

    compileResult.descriptorSetLayout = GetShaderPipeline ()->CreateDescriptorSetLayout (graphSettings.GetDevice ());

    uint32_t s = 0;

    graphSettings.connectionSet.ProcessInputBindingsOf (this, [&] (const IConnectionBinding& binding) {
        s += binding.GetLayerCount ();
    });

    s *= graphSettings.framesInFlight;

    if (s > 0) {
        compileResult.descriptorPool = std::make_unique<GVK::DescriptorPool> (graphSettings.GetDevice (), s, s, graphSettings.framesInFlight);

        for (uint32_t resourceIndex = 0; resourceIndex < graphSettings.framesInFlight; ++resourceIndex) {
            std::unique_ptr<GVK::DescriptorSet> descriptorSet = std::make_unique<GVK::DescriptorSet> (graphSettings.GetDevice (), *compileResult.descriptorPool, *compileResult.descriptorSetLayout);

            graphSettings.connectionSet.ProcessInputBindingsOf (this, [&] (const IConnectionBinding& binding) {
                binding.WriteToDescriptorSet (graphSettings.GetDevice (), *descriptorSet, resourceIndex);
            });

            compileResult.descriptorSets.push_back (std::move (descriptorSet));
        }
    }

    const auto attachmentReferences   = GetAttachmentReferences (graphSettings.connectionSet);
    const auto attachmentDescriptions = GetAttachmentDescriptions (graphSettings.connectionSet);

    if constexpr (IsDebugBuild) {
        GVK_ASSERT (attachmentReferences.size () == attachmentDescriptions.size ());
        for (uint32_t resourceIndex = 0; resourceIndex < graphSettings.framesInFlight; ++resourceIndex) {
            const auto outputImageView = GetOutputImageViews (graphSettings.connectionSet, resourceIndex);
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
                                                                             GetOutputImageViews (graphSettings.connectionSet, resourceIndex),
                                                                             //CreateOutputImageViews (graphSettings.GetDevice (), graphSettings.connectionSet, resourceIndex),
                                                                             width,
                                                                             height));
    }

    compileResult.width  = width;
    compileResult.height = height;
}


void RenderOperation::Record (const ConnectionSet& connectionSet, uint32_t resourceIndex, GVK::CommandBuffer& commandBuffer)
{
    uint32_t outputCount = 0;

    connectionSet.ProcessOutputBindingsOf (this, [&] (const IConnectionBinding& binding) {
        outputCount += binding.GetLayerCount ();
    });

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

    GVK_ASSERT (compileSettings.pureDrawRecordable != nullptr);
    compileSettings.pureDrawRecordable->Record (commandBuffer);
    
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

    connectionSet.VisitInputsOf (this, inputGatherer);
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


} // namespace RG
