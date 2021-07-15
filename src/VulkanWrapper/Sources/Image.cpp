#include "Image.hpp"

#include "Utils/Assert.hpp"

#include "spdlog/spdlog.h"

namespace GVK {

Image::Image (VkImage handle, VkDevice device, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, uint32_t arrayLayers)
    : handle (handle)
    , allocator (VK_NULL_HANDLE)
    , allocationHandle (VK_NULL_HANDLE)
    , device (device)
    , format (format)
    , width (width)
    , height (height)
    , depth (depth)
    , arrayLayers (arrayLayers)
{
}


Image::Image (VmaAllocator      allocator,
              VkImageType       imageType,
              uint32_t          width,
              uint32_t          height,
              uint32_t          depth,
              VkFormat          format,
              VkImageTiling     tiling,
              VkImageUsageFlags usage,
              uint32_t          arrayLayers,
              MemoryLocation    loc)
    : device (VK_NULL_HANDLE)
    , handle (VK_NULL_HANDLE)
    , allocator (allocator)
    , allocationHandle (VK_NULL_HANDLE)
    , format (format)
    , width (width)
    , height (height)
    , depth (depth)
    , arrayLayers (arrayLayers)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.flags             = 0;
    imageInfo.imageType         = imageType;
    imageInfo.extent.width      = width;
    imageInfo.extent.height     = height;
    imageInfo.extent.depth      = depth;
    imageInfo.mipLevels         = 1;
    imageInfo.arrayLayers       = arrayLayers;
    imageInfo.format            = format;
    imageInfo.tiling            = tiling;
    imageInfo.initialLayout     = INITIAL_LAYOUT;
    imageInfo.usage             = usage;
    imageInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage                   = (loc == MemoryLocation::GPU) ? VMA_MEMORY_USAGE_GPU_ONLY : VMA_MEMORY_USAGE_CPU_COPY;
    
    if (loc == MemoryLocation::CPU) {
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }

    if (GVK_ERROR (vmaCreateImage (allocator, &imageInfo, &allocInfo, &handle, &allocationHandle, nullptr) != VK_SUCCESS)) {
        spdlog::critical ("VkImage creation failed.");
        throw std::runtime_error ("failed to create image!");
    }

    spdlog::trace ("VkImage created: {}, uuid: {}.", handle, GetUUID ().GetValue ());
}


Image::Image (ImageBuilder& imageBuilder)
    : Image (imageBuilder.allocator,
             *imageBuilder.imageType,
             *imageBuilder.width,
             *imageBuilder.height,
             *imageBuilder.depth,
             *imageBuilder.format,
             *imageBuilder.tiling,
             imageBuilder.usage,
             *imageBuilder.arrayLayers,
             *imageBuilder.loc)
{
}


Image::~Image ()
{
    if (handle == VK_NULL_HANDLE) {
        return;
    }

    if (device != VK_NULL_HANDLE) {
        vkDestroyImage (device, handle, nullptr);
    } else if (allocator != VK_NULL_HANDLE) {
        vmaDestroyImage (allocator, handle, allocationHandle);
    }
    handle = nullptr;
}


static VkAccessFlags GetSrcAccessMask (VkImageLayout oldLayout)
{
    switch (oldLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED: return 0;
        case VK_IMAGE_LAYOUT_GENERAL: return VK_ACCESS_MEMORY_WRITE_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return VK_ACCESS_TRANSFER_READ_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return VK_ACCESS_TRANSFER_WRITE_BIT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_ACCESS_SHADER_READ_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        default:
            GVK_ERROR ("unhandled img layout transition");
            return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }
}


static VkAccessFlags GetDstAccessMask (VkImageLayout newLayout)
{
    GVK_ASSERT (newLayout != VK_IMAGE_LAYOUT_UNDEFINED);
    return GetSrcAccessMask (newLayout);
}


VkImageMemoryBarrier Image::GetBarrier (VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) const
{
    VkImageMemoryBarrier barrier = {};

    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = handle;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = arrayLayers;
    barrier.srcAccessMask                   = srcAccessMask;
    barrier.dstAccessMask                   = dstAccessMask;

    return barrier;
}


VkImageMemoryBarrier Image::GetBarrier (VkImageLayout oldLayout, VkImageLayout newLayout) const
{
    return GetBarrier (oldLayout, newLayout, GetSrcAccessMask (oldLayout), GetDstAccessMask (newLayout));
}


void Image::CmdPipelineBarrier (CommandBuffer& commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout) const
{
    VkImageMemoryBarrier barrier            = {};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = handle;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = arrayLayers;
    barrier.srcAccessMask                   = 0;
    barrier.dstAccessMask                   = 0;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        // TODO
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;
        sourceStage           = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        destinationStage      = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }


    sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    barrier.srcAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
                            VK_ACCESS_INDEX_READ_BIT |
                            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                            VK_ACCESS_UNIFORM_READ_BIT |
                            VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                            VK_ACCESS_SHADER_READ_BIT |
                            VK_ACCESS_SHADER_WRITE_BIT |
                            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                            VK_ACCESS_TRANSFER_READ_BIT |
                            VK_ACCESS_TRANSFER_WRITE_BIT |
                            VK_ACCESS_HOST_READ_BIT |
                            VK_ACCESS_HOST_WRITE_BIT;

    barrier.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
                            VK_ACCESS_INDEX_READ_BIT |
                            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                            VK_ACCESS_UNIFORM_READ_BIT |
                            VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                            VK_ACCESS_SHADER_READ_BIT |
                            VK_ACCESS_SHADER_WRITE_BIT |
                            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                            VK_ACCESS_TRANSFER_READ_BIT |
                            VK_ACCESS_TRANSFER_WRITE_BIT |
                            VK_ACCESS_HOST_READ_BIT |
                            VK_ACCESS_HOST_WRITE_BIT;

    VkMemoryBarrier flushAllMemory = {};
    flushAllMemory.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    flushAllMemory.srcAccessMask   = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
                                   VK_ACCESS_INDEX_READ_BIT |
                                   VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                                   VK_ACCESS_UNIFORM_READ_BIT |
                                   VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_SHADER_READ_BIT |
                                   VK_ACCESS_SHADER_WRITE_BIT |
                                   VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                   VK_ACCESS_TRANSFER_READ_BIT |
                                   VK_ACCESS_TRANSFER_WRITE_BIT |
                                       VK_ACCESS_HOST_READ_BIT |
                                   VK_ACCESS_HOST_WRITE_BIT;
    flushAllMemory.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
                                   VK_ACCESS_INDEX_READ_BIT |
                                   VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                                   VK_ACCESS_UNIFORM_READ_BIT |
                                   VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_SHADER_READ_BIT |
                                   VK_ACCESS_SHADER_WRITE_BIT |
                                   VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                   VK_ACCESS_TRANSFER_READ_BIT |
                                   VK_ACCESS_TRANSFER_WRITE_BIT |
                                       VK_ACCESS_HOST_READ_BIT |
                                   VK_ACCESS_HOST_WRITE_BIT;

    commandBuffer.Record<CommandPipelineBarrier> (
        sourceStage,
        destinationStage,
        std::vector<VkMemoryBarrier> { flushAllMemory },
        std::vector<VkBufferMemoryBarrier> {},
        std::vector<VkImageMemoryBarrier> { barrier });
}


VkBufferImageCopy Image::GetFullBufferImageCopy () const
{
    VkBufferImageCopy result               = {};
    result.bufferOffset                    = 0;
    result.bufferRowLength                 = 0;
    result.bufferImageHeight               = 0;
    result.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    result.imageSubresource.mipLevel       = 0;
    result.imageSubresource.baseArrayLayer = 0;
    result.imageSubresource.layerCount     = arrayLayers;
    result.imageOffset                     = { 0, 0, 0 };
    result.imageExtent                     = { width, height, depth };
    return result;
}


void Image::CmdCopyToBuffer (CommandBuffer& commandBuffer, VkBuffer buffer) const
{
    VkBufferImageCopy region = GetFullBufferImageCopy ();
    commandBuffer.Record<CommandCopyImageToBuffer> (
        handle,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        buffer,
        std::vector<VkBufferImageCopy> { region });
}


void Image::CmdCopyBufferToImage (CommandBuffer& commandBuffer, VkBuffer buffer) const
{
    VkBufferImageCopy region = GetFullBufferImageCopy ();
    commandBuffer.Record<CommandCopyBufferToImage> (
        buffer,
        handle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        std::vector<VkBufferImageCopy> { region });
}


void Image::CmdCopyBufferPartToImage (CommandBuffer& commandBuffer, VkBuffer buffer, VkBufferImageCopy region) const
{
    commandBuffer.Record<CommandGeneric> ([&] (VkCommandBuffer commandBuffer) {
        vkCmdCopyBufferToImage (
            commandBuffer,
            buffer,
            handle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);
    });
}

} // namespace GVK