#include "Resource.hpp"


namespace RenderGraph {


SingleImageResource::SingleImageResource (VkDevice device, Image::U&& image)
    : image (std::move (image))
    , imageView (ImageView::Create (device, *this->image.image, this->image.image->GetFormat ()))
    , sampler (Sampler::Create (device))
    , layoutRead (std::nullopt)
    , layoutWrite (VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
{
}


SingleImageResource::SingleImageResource (const GraphSettings& graphSettings, const Device& device, VkQueue queue, VkCommandPool commandPool, std::optional<VkImageLayout> layoutRead, std::optional<VkImageLayout> layoutWrite)
    : image (device, Image::Create (device, graphSettings.width, graphSettings.height, Format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT), DeviceMemory::GPU)
    , imageView (ImageView::Create (device, *image.image, image.image->GetFormat ()))
    , sampler (Sampler::Create (device))
    , layoutRead (layoutRead)
    , layoutWrite (layoutWrite)
{
}


void SingleImageResource::WriteToDescriptorSet (const DescriptorSet& descriptorSet, uint32_t binding) const
{
    descriptorSet.WriteOneImageInfo (
        binding,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        {*sampler, *imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
}


void SingleImageResource::BindRead (VkCommandBuffer commandBuffer)
{
    ASSERT (layoutRead.has_value ());
    VkImageLayout previousLayout = (layoutWrite.has_value ()) ? *layoutWrite : Image::INITIAL_LAYOUT;
    image.image->CmdTransitionImageLayout (commandBuffer, previousLayout, *layoutRead);
}


void SingleImageResource::BindWrite (VkCommandBuffer commandBuffer)
{
    ASSERT (layoutWrite.has_value ());
    image.image->CmdTransitionImageLayout (commandBuffer, Image::INITIAL_LAYOUT, *layoutWrite);
}


void ResourceVisitor::Visit (Resource&                               res,
                             VisitorCallback<ImageResource>          imageResourceTypeCallback,
                             VisitorCallback<SwapchainImageResource> swapchainImageResourceTypeCallback)
{
    ImageResource*          imageType          = dynamic_cast<ImageResource*> (&res);
    SwapchainImageResource* swapchainImageType = dynamic_cast<SwapchainImageResource*> (&res);
    if (imageType != nullptr) {
        imageResourceTypeCallback (*imageType);
    } else if (swapchainImageType != nullptr) {
        swapchainImageResourceTypeCallback (*swapchainImageType);
    } else {
        BREAK ("unexpected resource type");
        throw std::runtime_error ("unexpected resource type");
    }
}

} // namespace RenderGraph