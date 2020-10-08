#include "Operation.hpp"


namespace RG {


void Operation::AddInput (InputBindingU&& inputBinding)
{
    inputBindings.push_back (std::move (inputBinding));
}


void Operation::AddOutput (const uint32_t binding, const ImageResourceRef& res)
{
    GVK_ASSERT (std::find (outputBindings.begin (), outputBindings.end (), binding) == outputBindings.end ());

    for (uint32_t bindingIndex = binding; bindingIndex < binding + res.get ().GetDescriptorCount (); ++bindingIndex) {
        outputBindings.push_back (OutputBinding (bindingIndex, res.get ().GetFormat (), res.get ().GetFinalLayout ()));
    }
}


std::vector<VkAttachmentDescription> Operation::GetAttachmentDescriptions () const
{
    std::vector<VkAttachmentDescription> result;
    for (const auto& t : outputBindings) {
        result.push_back (t.attachmentDescription);
    }
    return result;
}


std::vector<VkAttachmentReference> Operation::GetAttachmentReferences () const
{
    std::vector<VkAttachmentReference> result;
    for (const auto& t : outputBindings) {
        result.push_back (t.attachmentReference);
    }
    return result;
}


std::vector<VkImageView> Operation::GetOutputImageViews (uint32_t frameIndex) const
{
    std::vector<VkImageView> result;

    ResourceVisitor v;

    v.onWritableImage = [&] (WritableImageResource& res) {
        for (auto& imgView : res.images[frameIndex]->imageViews) {
            result.push_back (*imgView);
        }
    };

    v.onSwapchainImage = [&] (SwapchainImageResource& res) {
        result.push_back (*res.imageViews[frameIndex]);
    };

    v.VisitAll (GetPointingTo<Resource> ());

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

    std::sort (inputBindings.begin (), inputBindings.end (), [&] (auto& a, auto& b) {
        return a->GetBinding () < b->GetBinding ();
    });

    std::vector<VkDescriptorSetLayoutBinding> layout;
    for (auto& ii : inputBindings) {
        layout.push_back (ii->ToDescriptorSetLayoutBinding ());
    }

    compileResult.descriptorSetLayout = DescriptorSetLayout::Create (graphSettings.GetDevice (), layout);

    uint32_t s = 0;
    for (auto& a : inputBindings) {
        s += a->GetLayerCount ();
    }
    s *= graphSettings.framesInFlight;

    if (s > 0) {
        compileResult.descriptorPool = DescriptorPool::Create (graphSettings.GetDevice (), s, s, graphSettings.framesInFlight);

        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            DescriptorSetU descriptorSet = DescriptorSet::Create (graphSettings.GetDevice (), *compileResult.descriptorPool, *compileResult.descriptorSetLayout);

            for (auto& ii : inputBindings) {
                ii->WriteToDescriptorSet (graphSettings.GetDevice (), *descriptorSet, frameIndex);
            }

            compileResult.descriptorSets.push_back (std::move (descriptorSet));
        }
    }

    const auto attachmentReferences   = GetAttachmentReferences ();
    const auto attachmentDescriptions = GetAttachmentDescriptions ();

    if constexpr (IsDebugBuild) {
        GVK_ASSERT (attachmentReferences.size () == attachmentDescriptions.size ());
        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            GVK_ASSERT (attachmentReferences.size () == GetOutputImageViews (frameIndex).size ());
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
        compileResult.framebuffers.push_back (Framebuffer::Create (graphSettings.GetDevice (), *compileSettings.pipeline->compileResult.renderPass, GetOutputImageViews (frameIndex), width, height));
    }

    compileResult.width  = width;
    compileResult.height = height;
}


void RenderOperation::Record (uint32_t frameIndex, CommandBuffer& commandBuffer)
{
    VkClearValue              clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    std::vector<VkClearValue> clearValues (outputBindings.size (), clearColor);

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass            = *compileSettings.pipeline->compileResult.renderPass;
    renderPassBeginInfo.framebuffer           = *compileResult.framebuffers[frameIndex];
    renderPassBeginInfo.renderArea.offset     = { 0, 0 };
    renderPassBeginInfo.renderArea.extent     = { compileResult.width, compileResult.height };
    renderPassBeginInfo.clearValueCount       = static_cast<uint32_t> (clearValues.size ());
    renderPassBeginInfo.pClearValues          = clearValues.data ();

    commandBuffer.CmdBeginRenderPass (&renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    commandBuffer.CmdBindPipeline (VK_PIPELINE_BIND_POINT_GRAPHICS, *compileSettings.pipeline->compileResult.pipeline);

    if (!compileResult.descriptorSets.empty ()) {
        VkDescriptorSet dsHandle = *compileResult.descriptorSets[frameIndex];

        commandBuffer.CmdBindDescriptorSets (VK_PIPELINE_BIND_POINT_GRAPHICS, *compileSettings.pipeline->compileResult.pipelineLayout, 0,
                                             1, &dsHandle,
                                             0, nullptr);
    }

    compileSettings.drawRecordable->Record (commandBuffer);

    commandBuffer.CmdEndRenderPass ();
}

} // namespace RG
