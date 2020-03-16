#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>


class Image : public Noncopyable {
private:
    const VkDevice device;
    const VkFormat format;
    VkImage        handle;
    uint32_t       width;
    uint32_t       height;

public:
    USING_PTR (Image);

    Image (VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage)
        : device (device)
        , handle (VK_NULL_HANDLE)
        , format (format)
        , width (width)
        , height (height)
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType         = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width      = width;
        imageInfo.extent.height     = height;
        imageInfo.extent.depth      = 1;
        imageInfo.mipLevels         = 1;
        imageInfo.arrayLayers       = 1;
        imageInfo.format            = format;
        imageInfo.tiling            = tiling;
        imageInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage             = usage;
        imageInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;

        if (ERROR (vkCreateImage (device, &imageInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create image!");
        }
    }

    ~Image ()
    {
        vkDestroyImage (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    VkFormat GetFormat () const { return format; }
    uint32_t GetWidth () const { return width; }
    uint32_t GetHeight () const { return height; }

    void CmdTransitionImageLayout (VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout) const
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
        barrier.subresourceRange.layerCount     = 1;
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

        vkCmdPipelineBarrier (
            commandBuffer,
            sourceStage,
            destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
    }


    operator VkImage () const { return handle; }
};

#endif