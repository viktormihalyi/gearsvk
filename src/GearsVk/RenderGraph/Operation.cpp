#include "Operation.hpp"


namespace RG {


void Operation::AddOutput (const uint32_t binding, const Resource::Ref& res)
{
    ASSERT (std::find (outputBindings.begin (), outputBindings.end (), binding) == outputBindings.end ());

    outputs.push_back (res);

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

    v.VisitAll (outputs);

    return result;
}


RenderOperation::RenderOperation (const DrawRecordable::P& drawRecordable, const ShaderPipeline::P& shaderPipeline)
    : compileSettings ({drawRecordable, shaderPipeline})
{
}

void RenderOperation::Compile (const GraphSettings& graphSettings)
{
    compileResult.Clear ();

    std::vector<VkDescriptorSetLayoutBinding> layout;
    for (auto& ii : newInputBindings) {
        layout.push_back (ii->ToDescriptorSetLayoutBinding ());
    }

    compileResult.descriptorSetLayout = DescriptorSetLayout::Create (graphSettings.GetDevice (), layout);

    const uint32_t s = newInputBindings.size () * graphSettings.framesInFlight;

    if (s > 0) {
        compileResult.descriptorPool = DescriptorPool::Create (graphSettings.GetDevice (), s, s, graphSettings.framesInFlight);

        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            DescriptorSet::U descriptorSet = DescriptorSet::Create (graphSettings.GetDevice (), *compileResult.descriptorPool, *compileResult.descriptorSetLayout);

            for (auto& ii : newInputBindings) {
                ii->WriteToDescriptorSet (graphSettings.GetDevice (), *descriptorSet, frameIndex);
            }

            compileResult.descriptorSets.push_back (std::move (descriptorSet));
        }
    }

    ASSERT ([&] () -> bool {
        ASSERT (GetAttachmentReferences ().size () == GetAttachmentDescriptions ().size ());
        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            ASSERT (GetAttachmentReferences ().size () == GetOutputImageViews (frameIndex).size ());
        }
        return true;
    }());

    compileSettings.pipeline->Compile ({graphSettings.width,
                                        graphSettings.height,
                                        *compileResult.descriptorSetLayout,
                                        GetAttachmentReferences (),
                                        GetAttachmentDescriptions (),
                                        compileSettings.drawRecordable->GetBindings (),
                                        compileSettings.drawRecordable->GetAttributes ()});

    for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
        compileResult.framebuffers.push_back (Framebuffer::Create (graphSettings.GetDevice (), *compileSettings.pipeline->compileResult.renderPass, GetOutputImageViews (frameIndex), graphSettings.width, graphSettings.height));
    }

    compileResult.width  = graphSettings.width;
    compileResult.height = graphSettings.height;
}


void RenderOperation::Record (uint32_t frameIndex, VkCommandBuffer commandBuffer)
{
    VkClearValue              clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    std::vector<VkClearValue> clearValues (outputBindings.size (), clearColor);

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass            = *compileSettings.pipeline->compileResult.renderPass;
    renderPassBeginInfo.framebuffer           = *compileResult.framebuffers[frameIndex];
    renderPassBeginInfo.renderArea.offset     = {0, 0};
    renderPassBeginInfo.renderArea.extent     = {compileResult.width, compileResult.height};
    renderPassBeginInfo.clearValueCount       = static_cast<uint32_t> (clearValues.size ());
    renderPassBeginInfo.pClearValues          = clearValues.data ();

    vkCmdBeginRenderPass (commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline (commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *compileSettings.pipeline->compileResult.pipeline);

    if (!compileResult.descriptorSets.empty ()) {
        VkDescriptorSet dsHandle = *compileResult.descriptorSets[frameIndex];

        vkCmdBindDescriptorSets (commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *compileSettings.pipeline->compileResult.pipelineLayout, 0,
                                 1, &dsHandle,
                                 0, nullptr);
    }

    compileSettings.drawRecordable->Record (commandBuffer);

    vkCmdEndRenderPass (commandBuffer);
}

} // namespace RG
