#include "Resource.hpp"


namespace RenderGraph {

ImageResource::ImageResource (const GraphInfo& graphInfo, const Device& device, VkQueue queue, VkCommandPool commandPool, std::optional<VkImageLayout> layoutRead, std::optional<VkImageLayout> layoutWrite)
    : image (device, Image::Create (device, graphInfo.width, graphInfo.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT), DeviceMemory::GPU)
    , imageView (ImageView::Create (device, *image.image, image.image->GetFormat ()))
    , sampler (Sampler::Create (device))
    , layoutRead (layoutRead)
    , layoutWrite (layoutWrite)
{
}


void ImageResource::WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const
{
    descriptorSet.WriteOneImageInfo (
        binding,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        {*sampler, *imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
}


void ImageResource::BindRead (VkCommandBuffer commandBuffer)
{
    ASSERT (layoutRead.has_value ());
    VkImageLayout previousLayout = (layoutWrite.has_value ()) ? *layoutWrite : Image::INITIAL_LAYOUT;
    image.image->CmdTransitionImageLayout (commandBuffer, previousLayout, *layoutRead);
}


void ImageResource::BindWrite (VkCommandBuffer commandBuffer)
{
    ASSERT (layoutWrite.has_value ());
    image.image->CmdTransitionImageLayout (commandBuffer, Image::INITIAL_LAYOUT, *layoutWrite);
}


void ResourceVisitor::Visit (Resource& res, const std::function<void (ImageResource&)>& imageResourceTypeCallback)
{
    ImageResource* imageType = dynamic_cast<ImageResource*> (&res);
    if (imageType != nullptr) {
        imageResourceTypeCallback (*imageType);
    } else {
        BREAK ("unexpected resource type");
        throw std::runtime_error ("unexpected resource type");
    }
}

} // namespace RenderGraph