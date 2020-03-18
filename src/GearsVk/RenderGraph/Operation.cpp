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


OutputBinding::OutputBinding (uint32_t binding)
    : binding (binding)
    , attachmentDescription ({})
    , attachmentReference ({})
{
    attachmentDescription.format         = VK_FORMAT_R8G8B8A8_SRGB;
    attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout  = Image::INITIAL_LAYOUT;                    // TODO
    attachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // TODO

    attachmentReference.attachment = binding;
    attachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // TODO
}


void Operation::AddInput (uint32_t binding, const Resource::Ref& res)
{
    ASSERT (std::find (inputBindings.begin (), inputBindings.end (), binding) == inputBindings.end ());

    inputs.push_back (res);
    inputBindings.push_back (binding);
}

void Operation::AddOutput (uint32_t binding, const Resource::Ref& res)
{
    ASSERT (std::find (outputBindings.begin (), outputBindings.end (), binding) == outputBindings.end ());

    outputs.push_back (res);
    outputBindings.push_back (binding);
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

std::vector<VkImageView> Operation::GetOutputImageViews () const
{
    std::vector<VkImageView> result;

    auto onImage = [&] (ImageResource& res) {
        result.push_back (*res.imageView);
    };

    for (const auto& o : outputs) {
        ResourceVisitor::Visit (o, onImage);
    }

    return result;
}


LambdaOperation::LambdaOperation (const GraphInfo& graphInfo, VkDevice device, VkCommandPool commandPool, const std::vector<std::filesystem::path>& shaders,
                                  const std::function<void ()>&               compileFunc,
                                  const std::function<void (VkCommandBuffer)> recordFunc)
    : graphInfo (graphInfo)
    , compileFunc (compileFunc)
    , recordFunc (recordFunc)
{
    ASSERT (!shaders.empty ());

    pipeline.AddShaders (device, shaders);
}


RenderOperation::RenderOperation (const GraphInfo& graphInfo, VkDevice device, VkCommandPool commandPool, uint32_t vertexCount, const std::vector<std::filesystem::path>& shaders)
    : graphInfo (graphInfo)
    , device (device)
    , vertexCount (vertexCount)
{
    ASSERT (!shaders.empty ());


    std::cout << "compiling ";
    for (auto& s : shaders) {
        std::cout << s.u8string () << " ";
    }
    std::cout << "... ";

    pipeline.AddShaders (device, shaders);

    std::cout << "done" << std::endl;
}


void RenderOperation::Compile ()
{
    std::vector<VkDescriptorSetLayoutBinding> layout;
    for (auto& inputBinding : inputBindings) {
        layout.push_back (inputBinding.descriptor);
    }

    descriptorSetLayout = DescriptorSetLayout::Create (device, layout);

    if (!inputBindings.empty ()) {
        descriptorPool = DescriptorPool::Create (device, 0, inputBindings.size (), 1);
        descriptorSet  = DescriptorSet::Create (device, *descriptorPool, *descriptorSetLayout);

        for (uint32_t i = 0; i < inputs.size (); ++i) {
            Resource& r = inputs[i];
            r.WriteToDescriptorSet (*descriptorSet, inputBindings[i].binding);
        }
    }

    pipeline.Compile (device, graphInfo.width, graphInfo.height, *descriptorSetLayout, GetAttachmentReferences (), GetAttachmentDescriptions ());

    framebuffer = Framebuffer::Create (device, *pipeline.renderPass, GetOutputImageViews (), graphInfo.width, graphInfo.height);
}


void RenderOperation::Record (VkCommandBuffer commandBuffer)
{
    VkClearValue              clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    std::vector<VkClearValue> clearValues (outputs.size (), clearColor);

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass            = *pipeline.renderPass;
    renderPassBeginInfo.framebuffer           = *framebuffer;
    renderPassBeginInfo.renderArea.offset     = {0, 0};
    renderPassBeginInfo.renderArea.extent     = {graphInfo.width, graphInfo.height};
    renderPassBeginInfo.clearValueCount       = clearValues.size ();
    renderPassBeginInfo.pClearValues          = clearValues.data ();

    vkCmdBeginRenderPass (commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline (commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline.pipeline);

    if (descriptorSet) {
        VkDescriptorSet dsHandle = *descriptorSet;

        vkCmdBindDescriptorSets (commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline.pipelineLayout, 0,
                                 1, &dsHandle,
                                 0, nullptr);
    }
    vkCmdDraw (commandBuffer, vertexCount, 1, 0, 0);
    vkCmdEndRenderPass (commandBuffer);
}

} // namespace RenderGraph
