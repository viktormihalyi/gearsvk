#include "WritableImageResource.hpp"


namespace RG {


struct WritableImageResource::SingleImageResource final {
    USING_CREATE (SingleImageResource);

    static const VkFormat FormatRGBA;
    static const VkFormat FormatRGB;

    const ImageBaseU             image;
    std::vector<ImageView2DU>    imageViews;
    std::optional<VkImageLayout> layoutRead;
    std::optional<VkImageLayout> layoutWrite;

    // write always happens before read
    // NO  read, NO  write: general
    // YES read, NO  write: general -> read
    // NO  read, YES write: general -> write
    // YES read, YES write: general -> write -> read

    SingleImageResource (const GraphSettings& graphSettings, uint32_t width, uint32_t height, uint32_t arrayLayers)
        : image (Image2D::Create (graphSettings.GetDevice ().GetAllocator (), ImageBase::MemoryLocation::GPU, width, height, FormatRGBA, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, arrayLayers))
    {
        for (uint32_t layerIndex = 0; layerIndex < arrayLayers; ++layerIndex) {
            imageViews.push_back (ImageView2D::Create (graphSettings.GetDevice (), *image, layerIndex));
        }
    }
};


WritableImageResource::WritableImageResource (uint32_t width, uint32_t height, uint32_t arrayLayers)
    : width (width)
    , height (height)
    , arrayLayers (arrayLayers)
{
}


void WritableImageResource::BindRead (uint32_t imageIndex, CommandBuffer& commandBuffer)
{
    SingleImageResource& im = *images[imageIndex];

    im.layoutRead                = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkImageLayout previousLayout = (im.layoutWrite.has_value ()) ? *im.layoutWrite : ImageBase::INITIAL_LAYOUT;
    im.image->CmdPipelineBarrier (commandBuffer, previousLayout, *im.layoutRead);
}


void WritableImageResource::BindWrite (uint32_t imageIndex, CommandBuffer& commandBuffer)
{
    SingleImageResource& im = *images[imageIndex];

    im.layoutWrite = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    im.image->CmdPipelineBarrier (commandBuffer, ImageBase::INITIAL_LAYOUT, *im.layoutWrite);
}


void WritableImageResource::Compile (const GraphSettings& graphSettings)
{
    sampler = Sampler::Create (graphSettings.GetDevice (), VK_FILTER_LINEAR);

    images.clear ();
    for (uint32_t frameIndex = 0; frameIndex < graphSettings.framesInFlight; ++frameIndex) {
        images.push_back (SingleImageResource::Create (graphSettings, width, height, arrayLayers));
    }
}


VkImageLayout WritableImageResource::GetFinalLayout () const
{
    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}


VkFormat WritableImageResource::GetFormat () const
{
    return SingleImageResource::FormatRGBA;
}


uint32_t WritableImageResource::GetDescriptorCount () const
{
    return arrayLayers;
}


std::vector<ImageBase*> WritableImageResource::GetImages () const
{
    std::vector<ImageBase*> result;

    for (auto& img : images) {
        result.push_back (img->image.get ());
    }

    return result;
}


VkImageView WritableImageResource::GetImageViewForFrame (uint32_t frameIndex, uint32_t layerIndex)
{
    return *images[frameIndex]->imageViews[layerIndex];
}


VkSampler WritableImageResource::GetSampler ()
{
    return *sampler;
}


} // namespace RG
