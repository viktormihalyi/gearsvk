#include "Resource.hpp"


namespace RG {


// sampled formats should always be _SRGB?
const VkFormat SingleImageResource::FormatRGBA = VK_FORMAT_R8G8B8A8_SRGB;
const VkFormat SingleImageResource::FormatRGB  = VK_FORMAT_R8G8B8_SRGB;


SingleImageResource::SingleImageResource (const GraphSettings& graphSettings, uint32_t arrayLayers)
    : image (graphSettings.GetDevice (), Image2D::Create (graphSettings.GetDevice (), graphSettings.width, graphSettings.height, FormatRGBA, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, arrayLayers), DeviceMemory::GPU)
    , sampler (Sampler::Create (graphSettings.GetDevice ()))
{
    for (uint32_t layerIndex = 0; layerIndex < arrayLayers; ++layerIndex) {
        imageViews.push_back (ImageView2D::Create (graphSettings.GetDevice (), *image.image, layerIndex));
    }
}


void SingleImageResource::BindRead (VkCommandBuffer commandBuffer)
{
    layoutRead                   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkImageLayout previousLayout = (layoutWrite.has_value ()) ? *layoutWrite : Image2D::INITIAL_LAYOUT;
    image.image->CmdPipelineBarrier (commandBuffer, previousLayout, *layoutRead);
}


void SingleImageResource::BindWrite (VkCommandBuffer commandBuffer)
{
    layoutWrite = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    image.image->CmdPipelineBarrier (commandBuffer, Image2D::INITIAL_LAYOUT, *layoutWrite);
}


#define ADDVISITOR(type, callback)                   \
    if (auto castedRes = dynamic_cast<type*> (&res)) \
        callback (*castedRes);                       \
    else


void ResourceVisitor::Visit (Resource& res)
{
    ADDVISITOR (WritableImageResource, onWritableImage)
    ADDVISITOR (ReadOnlyImageResource, onReadOnlyImage)
    ADDVISITOR (SwapchainImageResource, onSwapchainImage)
    ADDVISITOR (UniformBlockResource, onUniformImage)
    {
        BREAK ("unexpected resource type");
        throw std::runtime_error ("unexpected resource type");
    }
}

} // namespace RG