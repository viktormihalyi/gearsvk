#include "Operation.hpp"


namespace RG {


std::vector<VkAttachmentDescription> RenderOperation::GetAttachmentDescriptions (const ConnectionSet& conncetionSet) const
{
    std::vector<VkAttachmentDescription> result;

    IConnectionBindingVisitorFn visitor (
        [] (UniformInputBinding&) {},
        [] (ImageInputBinding&) {},
        [&] (OutputBinding& binding) {
            for (auto& a : binding.GetAttachmentDescriptions ()) {
                result.push_back (a);
            }
        });

    conncetionSet.VisitOutputsOf (this, visitor);

    return result;
}

std::vector<VkAttachmentReference> RenderOperation::GetAttachmentReferences (const ConnectionSet& conncetionSet) const
{
    std::vector<VkAttachmentReference> result;

    IConnectionBindingVisitorFn visitor (
        [] (UniformInputBinding&) {},
        [] (ImageInputBinding&) {},
        [&] (OutputBinding& binding) {
            for (auto& a : binding.GetAttachmentReferences ()) {
                result.push_back (a);
            }
        });

    conncetionSet.VisitOutputsOf (this, visitor);

    return result;
}


std::vector<VkImageView> RenderOperation::GetOutputImageViews (const ConnectionSet& conncetionSet, uint32_t frameIndex) const
{
    std::vector<VkImageView> result;

    IResourceVisitorFn outputGatherer ([] (ReadOnlyImageResource&) {},
                                       [&] (WritableImageResource& res) {
        for (auto& imgView : res.images[frameIndex]->imageViews) {
            result.push_back (*imgView);
        } },
                                       [&] (SwapchainImageResource& res) {
                                           result.push_back (*res.imageViews[frameIndex]);
                                       },
                                       [] (GPUBufferResource&) {},
                                       [] (CPUBufferResource&) {});


    conncetionSet.VisitOutputsOf (this, outputGatherer);


    return result;
}


RenderOperation::RenderOperation (const DrawRecordableP& drawRecordable, const ShaderPipelineP& shaderPipeline, VkPrimitiveTopology topology)
    : compileSettings ({ drawRecordable, drawRecordable, shaderPipeline, topology })
{
}


RenderOperation::RenderOperation (const PureDrawRecordableP& drawRecordable, const VertexAttributeProviderP& vap, const ShaderPipelineP& shaderPipeline, VkPrimitiveTopology topology)
    : compileSettings ({ drawRecordable, vap, shaderPipeline, topology })
{
}


void RenderOperation::Compile (const GraphSettings& graphSettings, uint32_t width, uint32_t height)
{
    compileResult.Clear ();

    std::vector<VkDescriptorSetLayoutBinding> layout;

    IConnectionBindingVisitorFn layoutBindingGatherer (
        [&] (const UniformInputBinding& binding) {
            layout.push_back (binding.ToDescriptorSetLayoutBinding ());
        },
        [&] (const ImageInputBinding& binding) {
            layout.push_back (binding.ToDescriptorSetLayoutBinding ());
        },
        [] (const OutputBinding& binding) {});

    graphSettings.connectionSet.VisitInputsOf (this, layoutBindingGatherer);

    compileResult.descriptorSetLayout = DescriptorSetLayout::Create (graphSettings.GetDevice (), layout);

    uint32_t s = 0;

    IConnectionBindingVisitorFn layerCountAdder (
        [&] (const UniformInputBinding& binding) {
            s += binding.GetLayerCount ();
        },
        [&] (const ImageInputBinding& binding) {
            s += binding.GetLayerCount ();
        },
        [] (const OutputBinding& binding) {});

    graphSettings.connectionSet.VisitInputsOf (this, layerCountAdder);

    s *= graphSettings.framesInFlight;

    if (s > 0) {
        compileResult.descriptorPool = DescriptorPool::Create (graphSettings.GetDevice (), s, s, graphSettings.framesInFlight);

        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            DescriptorSetU descriptorSet = DescriptorSet::Create (graphSettings.GetDevice (), *compileResult.descriptorPool, *compileResult.descriptorSetLayout);

            IConnectionBindingVisitorFn layerCountAdder (
                [&] (const UniformInputBinding& binding) {
                    binding.WriteToDescriptorSet (graphSettings.GetDevice (), *descriptorSet, frameIndex);
                },
                [&] (const ImageInputBinding& binding) {
                    binding.WriteToDescriptorSet (graphSettings.GetDevice (), *descriptorSet, frameIndex);
                },
                [] (const OutputBinding& binding) {});

            graphSettings.connectionSet.VisitInputsOf (this, layerCountAdder);


            compileResult.descriptorSets.push_back (std::move (descriptorSet));
        }
    }

    const auto attachmentReferences   = GetAttachmentReferences (graphSettings.connectionSet);
    const auto attachmentDescriptions = GetAttachmentDescriptions (graphSettings.connectionSet);

    if constexpr (IsDebugBuild) {
        GVK_ASSERT (attachmentReferences.size () == attachmentDescriptions.size ());
        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            const auto outputImageView = GetOutputImageViews (graphSettings.connectionSet, frameIndex);
            GVK_ASSERT (attachmentReferences.size () == outputImageView.size ());
        }
    }

    ShaderPipeline::CompileSettings pipelineSettigns = { width,
                                                         height,
                                                         *compileResult.descriptorSetLayout,
                                                         attachmentReferences,
                                                         attachmentDescriptions,
                                                         compileSettings.vertexAttributeProvider->GetBindings (),
                                                         compileSettings.vertexAttributeProvider->GetAttributes (),
                                                         compileSettings.topology };

    compileSettings.pipeline->Compile (pipelineSettigns);

    for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
        compileResult.framebuffers.push_back (Framebuffer::Create (graphSettings.GetDevice (), *compileSettings.pipeline->compileResult.renderPass, GetOutputImageViews (graphSettings.connectionSet, frameIndex), width, height));
    }

    compileResult.width  = width;
    compileResult.height = height;
}


void RenderOperation::Record (const ConnectionSet& connectionSet, uint32_t frameIndex, CommandBuffer& commandBuffer)
{
    uint32_t outputCount = 0;

    IConnectionBindingVisitorFn outputCounter (
        [] (const UniformInputBinding&) {
        },
        [] (const ImageInputBinding&) {
        },
        [&] (const OutputBinding& binding) {
            outputCount += binding.layerCount;
        });

    connectionSet.VisitOutputsOf (this, outputCounter);


    VkClearValue              clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    std::vector<VkClearValue> clearValues (outputCount, clearColor);

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass            = *compileSettings.pipeline->compileResult.renderPass;
    renderPassBeginInfo.framebuffer           = *compileResult.framebuffers[frameIndex];
    renderPassBeginInfo.renderArea.offset     = { 0, 0 };
    renderPassBeginInfo.renderArea.extent     = { compileResult.width, compileResult.height };
    renderPassBeginInfo.clearValueCount       = static_cast<uint32_t> (clearValues.size ());
    renderPassBeginInfo.pClearValues          = clearValues.data ();

    // TODO transition image layouts here

    //for (auto& i : inputBindings) {
    //    i.RecordTransition (commandBuffer);
    //}

    //for (auto& o : outputBindings) {
    //    o.RecordTransition (commandBuffer);
    //}

    // inputs:  undef/color_attachment_optimal ?? -> shader_read_only_optimal
    // outputs: undef ?? -> color_attachment_optimal

    commandBuffer.RecordT<CommandGeneric> ([&] (VkCommandBuffer commandBuffer) { vkCmdBeginRenderPass (commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); });

    commandBuffer.RecordT<CommandGeneric> ([&] (VkCommandBuffer commandBuffer) { vkCmdBindPipeline (commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *compileSettings.pipeline->compileResult.pipeline); });

    if (!compileResult.descriptorSets.empty ()) {
        VkDescriptorSet dsHandle = *compileResult.descriptorSets[frameIndex];

        commandBuffer.RecordT<CommandGeneric> ([&] (VkCommandBuffer commandBuffer) {
            vkCmdBindDescriptorSets (commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *compileSettings.pipeline->compileResult.pipelineLayout, 0,
                                     1, &dsHandle,
                                     0, nullptr);
        });
    }

    compileSettings.drawRecordable->Record (commandBuffer);

    commandBuffer.RecordT<CommandGeneric> ([&] (VkCommandBuffer commandBuffer) { vkCmdEndRenderPass (commandBuffer); });
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


void TransferOperation::Record (const ConnectionSet& connectionSet, uint32_t imageIndex, CommandBuffer& commandBuffer)
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
            ImageBase* fromVkImage = fromImg->GetImages ()[0];
            ImageBase* toVkImage   = toImg->GetImages ()[0];

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

            commandBuffer.RecordT<CommandGeneric> ([&] (VkCommandBuffer commandBuffer) {
                vkCmdCopyImage (
                    commandBuffer,
                    *fromVkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    *toVkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &imageCopyRegion);
            });
            return;
        }
    }

    GVK_ASSERT ("bad resources");
    throw std::runtime_error ("unknown resources to transfer");
}


} // namespace RG
