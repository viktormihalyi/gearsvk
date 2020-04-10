#include "Resource.hpp"


namespace RenderGraphns {


SingleImageResource::SingleImageResource (const GraphSettings& graphSettings, uint32_t arrayLayers)
    : image (graphSettings.GetDevice (), Image::Create (graphSettings.GetDevice (), graphSettings.width, graphSettings.height, Format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, arrayLayers), DeviceMemory::GPU)
    , sampler (Sampler::Create (graphSettings.GetDevice ()))
{
    for (uint32_t layerIndex = 0; layerIndex < arrayLayers; ++layerIndex) {
        imageViews.push_back (ImageView::Create (graphSettings.GetDevice (), *image.image, layerIndex));
    }
}


void SingleImageResource::WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const
{
    for (auto& imgView : imageViews) {
        descriptorSet.WriteOneImageInfo (
            binding,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            {*sampler, *imgView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    }
}


void SingleImageResource::BindRead (VkCommandBuffer commandBuffer)
{
    layoutRead                   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkImageLayout previousLayout = (layoutWrite.has_value ()) ? *layoutWrite : Image::INITIAL_LAYOUT;
    image.image->CmdPipelineBarrier (commandBuffer, previousLayout, *layoutRead);
}


void SingleImageResource::BindWrite (VkCommandBuffer commandBuffer)
{
    layoutWrite = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    image.image->CmdPipelineBarrier (commandBuffer, Image::INITIAL_LAYOUT, *layoutWrite);
}


void ResourceVisitor::Visit (Resource&                               res,
                             VisitorCallback<ImageResource>          imageResourceTypeCallback,
                             VisitorCallback<SwapchainImageResource> swapchainImageResourceTypeCallback,
                             VisitorCallback<UniformBlockResource>   uniformBlockResourceTypeCallback)
{
    ImageResource*          imageType          = dynamic_cast<ImageResource*> (&res);
    SwapchainImageResource* swapchainImageType = dynamic_cast<SwapchainImageResource*> (&res);
    UniformBlockResource*   uniformBlockType   = dynamic_cast<UniformBlockResource*> (&res);
    if (imageType != nullptr) {
        imageResourceTypeCallback (*imageType);
    } else if (swapchainImageType != nullptr) {
        swapchainImageResourceTypeCallback (*swapchainImageType);
    } else if (uniformBlockType != nullptr) {
        uniformBlockResourceTypeCallback (*uniformBlockType);
    } else {
        BREAK ("unexpected resource type");
        throw std::runtime_error ("unexpected resource type");
    }
}

} // namespace RenderGraphns