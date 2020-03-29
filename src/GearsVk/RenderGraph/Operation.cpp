#include "Operation.hpp"


namespace RenderGraph {

void Operation::AddInput (const uint32_t binding, const Resource::Ref& res)
{
    const InputBinding newBinding (binding, res.get ().GetDescriptorType (), res.get ().GetDescriptorCount ());

    ASSERT (std::find (inputBindings.begin (), inputBindings.end (), newBinding) == inputBindings.end ());

    inputs.push_back (res);
    inputBindings.push_back (newBinding);
}


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

    auto onImage = [&] (ImageResource& res) {
        for (auto& imgView : res.images[frameIndex]->imageViews) {
            result.push_back (*imgView);
        }
    };

    auto onSwapchainImage = [&] (SwapchainImageResource& res) {
        result.push_back (*res.imageViews[frameIndex]);
    };

    auto Swallow = [] (auto&... res) {};

    for (const auto& o : outputs) {
        ResourceVisitor::Visit (o, onImage, onSwapchainImage, Swallow);
    }

    return result;
}


RenderOperation::RenderOperation (const GraphSettings& graphSettings, const RenderOperationSettings& settings, const std::vector<std::filesystem::path>& shaders)
    : graphSettings (graphSettings)
    , settings (settings)
{
    ASSERT (!shaders.empty ());


    std::cout << "compiling ";
    for (auto& s : shaders) {
        std::cout << s.u8string () << " ";
    }
    std::cout << "... ";

    pipeline = ShaderPipeline::Create (graphSettings.device);
    pipeline->AddShaders (shaders);

    std::cout << "done" << std::endl;
}


RenderOperation::RenderOperation (const GraphSettings& graphSettings, const RenderOperationSettings& settings, ShaderPipeline::U&& shaderPipeline)
    : graphSettings (graphSettings)
    , pipeline (std::move (shaderPipeline))
    , settings (settings)
{
}


void RenderOperation::Compile ()
{
    std::vector<VkDescriptorSetLayoutBinding> layout;
    for (auto& inputBinding : inputBindings) {
        layout.push_back (inputBinding);
    }

    descriptorSetLayout = DescriptorSetLayout::Create (graphSettings.device, layout);

    if (!inputBindings.empty ()) {
        descriptorPool = DescriptorPool::Create (graphSettings.device, 0, inputBindings.size () * graphSettings.framesInFlight, graphSettings.framesInFlight);

        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            DescriptorSet::U descriptorSet = DescriptorSet::Create (graphSettings.device, *descriptorPool, *descriptorSetLayout);

            for (uint32_t i = 0; i < inputs.size (); ++i) {
                Resource& r = inputs[i];
                r.WriteToDescriptorSet (frameIndex, *descriptorSet, inputBindings[i].binding);
            }

            descriptorSets.push_back (std::move (descriptorSet));
        }
    }

    ASSERT ([&] () -> bool {
        ASSERT (GetAttachmentReferences ().size () == GetAttachmentDescriptions ().size ());
        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            ASSERT (GetAttachmentReferences ().size () == GetOutputImageViews (frameIndex).size ());
        }
        return true;
    }());

    pipeline->Compile ({graphSettings.width, graphSettings.height, *descriptorSetLayout, GetAttachmentReferences (), GetAttachmentDescriptions (), settings.vertexInputBindings, settings.vertexInputAttributes});

    for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
        framebuffers.push_back (Framebuffer::Create (graphSettings.device, *pipeline->renderPass, GetOutputImageViews (frameIndex), graphSettings.width, graphSettings.height));
    }
}


void RenderOperation::Record (uint32_t frameIndex, VkCommandBuffer commandBuffer)
{
    VkClearValue              clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    std::vector<VkClearValue> clearValues (outputBindings.size (), clearColor);

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass            = *pipeline->renderPass;
    renderPassBeginInfo.framebuffer           = *framebuffers[frameIndex];
    renderPassBeginInfo.renderArea.offset     = {0, 0};
    renderPassBeginInfo.renderArea.extent     = {graphSettings.width, graphSettings.height};
    renderPassBeginInfo.clearValueCount       = clearValues.size ();
    renderPassBeginInfo.pClearValues          = clearValues.data ();

    vkCmdBeginRenderPass (commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline (commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->pipeline);

    if (settings.vertexBuffer != VK_NULL_HANDLE) {
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers (commandBuffer, 0, 1, &settings.vertexBuffer, offsets);
    }

    if (settings.indexBuffer != VK_NULL_HANDLE) {
        vkCmdBindIndexBuffer (commandBuffer, settings.indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    }

    if (!descriptorSets.empty ()) {
        VkDescriptorSet dsHandle = *descriptorSets[frameIndex];

        vkCmdBindDescriptorSets (commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->pipelineLayout, 0,
                                 1, &dsHandle,
                                 0, nullptr);
    }


    if (settings.indexBuffer != VK_NULL_HANDLE) {
        vkCmdDrawIndexed (commandBuffer, settings.indexCount, settings.instanceCount, 0, 0, 0);
    } else {
        vkCmdDraw (commandBuffer, settings.vertexCount, settings.instanceCount, 0, 0);
    }

    vkCmdEndRenderPass (commandBuffer);
}

} // namespace RenderGraph
