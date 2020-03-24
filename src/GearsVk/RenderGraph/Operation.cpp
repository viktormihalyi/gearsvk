#include "Operation.hpp"


namespace RenderGraph {

InputBinding::InputBinding (uint32_t binding)
    : binding (binding)
{
    descriptor.binding            = binding;
    descriptor.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor.descriptorCount    = 1;
    descriptor.stageFlags         = VK_SHADER_STAGE_ALL_GRAPHICS;
    descriptor.pImmutableSamplers = nullptr;
}


OutputBinding::OutputBinding (uint32_t binding, VkFormat format, VkImageLayout finalLayout)
    : binding (binding)
    , attachmentDescription ({})
    , attachmentReference ({})
    , format (format)
    , finalLayout (finalLayout)
{
    attachmentDescription.format         = format;
    attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout  = Image::INITIAL_LAYOUT; // TODO
    attachmentDescription.finalLayout    = finalLayout;

    attachmentReference.attachment = binding;
    attachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}


void Operation::AddInput (uint32_t binding, const Resource::Ref& res)
{
    ASSERT (std::find (inputBindings.begin (), inputBindings.end (), binding) == inputBindings.end ());

    inputs.push_back (res);
    inputBindings.push_back (InputBinding (binding));
}


void Operation::AddOutput (uint32_t binding, VkFormat format, VkImageLayout finalLayout, const Resource::Ref& res)
{
    ASSERT (std::find (outputBindings.begin (), outputBindings.end (), binding) == outputBindings.end ());

    outputs.push_back (res);
    outputBindings.push_back (OutputBinding (binding, format, finalLayout));
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
        result.push_back (*res.images[frameIndex]->imageView);
    };

    auto onSwapchainImage = [&] (SwapchainImageResource& res) {
        result.push_back (*res.imageViews[frameIndex]);
    };

    for (const auto& o : outputs) {
        ResourceVisitor::Visit (o, onImage, onSwapchainImage);
    }

    return result;
}
//
//
//LambdaOperation::LambdaOperation (const GraphInfo& graphSettings, VkDevice device, VkCommandPool commandPool, const std::vector<std::filesystem::path>& shaders,
//                                  const std::function<void ()>&               compileFunc,
//                                  const std::function<void (VkCommandBuffer)> recordFunc)
//    : graphSettings (graphSettings)
//    , compileFunc (compileFunc)
//    , recordFunc (recordFunc)
//{
//    ASSERT (!shaders.empty ());
//
//    pipeline.AddShaders (device, shaders);
//}


RenderOperation::RenderOperation (const GraphSettings& graphSettings, VkDevice device, VkCommandPool commandPool, uint32_t vertexCount, const std::vector<std::filesystem::path>& shaders)
    : graphSettings (graphSettings)
    , device (device)
    , vertexCount (vertexCount)
{
    ASSERT (!shaders.empty ());


    std::cout << "compiling ";
    for (auto& s : shaders) {
        std::cout << s.u8string () << " ";
    }
    std::cout << "... ";

    pipeline = ShaderPipeline::Create (device);
    pipeline->AddShaders (device, shaders);

    std::cout << "done" << std::endl;
}


RenderOperation::RenderOperation (const GraphSettings& graphSettings, VkDevice device, VkCommandPool commandPool, uint32_t vertexCount, ShaderPipeline::U&& shaderPipeline)
    : graphSettings (graphSettings)
    , device (device)
    , vertexCount (vertexCount)
    , pipeline (std::move (shaderPipeline))
{
}


void RenderOperation::Compile ()
{
    std::vector<VkDescriptorSetLayoutBinding> layout;
    for (auto& inputBinding : inputBindings) {
        layout.push_back (inputBinding.descriptor);
    }

    descriptorSetLayout = DescriptorSetLayout::Create (device, layout);

    if (!inputBindings.empty ()) {
        descriptorPool = DescriptorPool::Create (device, 0, inputBindings.size () * graphSettings.framesInFlight, graphSettings.framesInFlight);

        for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
            DescriptorSet::U descriptorSet = DescriptorSet::Create (device, *descriptorPool, *descriptorSetLayout);

            for (uint32_t i = 0; i < inputs.size (); ++i) {
                Resource& r = inputs[i];
                r.WriteToDescriptorSet (frameIndex, *descriptorSet, inputBindings[i].binding);
            }

            descriptorSets.push_back (std::move (descriptorSet));
        }
    }

    pipeline->Compile (device, graphSettings.width, graphSettings.height, *descriptorSetLayout, GetAttachmentReferences (), GetAttachmentDescriptions ());

    for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
        framebuffers.push_back (Framebuffer::Create (device, *pipeline->renderPass, GetOutputImageViews (frameIndex), graphSettings.width, graphSettings.height));
    }
}


void RenderOperation::Record (uint32_t frameIndex, VkCommandBuffer commandBuffer)
{
    VkClearValue              clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    std::vector<VkClearValue> clearValues (outputs.size (), clearColor);

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

    if (!descriptorSets.empty ()) {
        VkDescriptorSet dsHandle = *descriptorSets[frameIndex];

        vkCmdBindDescriptorSets (commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->pipelineLayout, 0,
                                 1, &dsHandle,
                                 0, nullptr);
    }

    vkCmdDraw (commandBuffer, vertexCount, 1, 0, 0);

    vkCmdEndRenderPass (commandBuffer);
}

} // namespace RenderGraph
