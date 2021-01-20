#include "CommandBuffer.hpp"
#include "Image.hpp"

#include <iostream>


void CommandBuffer::ImageLayoutChanged (const Image& image, VkImageLayout from, VkImageLayout to)
{
    const Image* img = &image;
    layouts[img].emplace_back (from, to);
}


void CommandBindVertexBuffers::Record (CommandBuffer& commandBuffer)
{
    vkCmdBindVertexBuffers (commandBuffer.GetHandle (), firstBinding, bindingCount, pBuffers.data (), pOffsets.data ());
}


std::string CommandBindVertexBuffers::ToString () const
{
    return std::string ("vkCmdBindVertexBuffers (") + std::to_string (firstBinding) + ", " + std::to_string (bindingCount) + ", [buffers], [offsets])";
}


void CommandPipelineBarrier::Record (CommandBuffer& commandBuffer)
{
    vkCmdPipelineBarrier (
        commandBuffer.GetHandle (),
        srcStageMask,
        dstStageMask,
        0, // TODO
        static_cast<uint32_t> (memoryBarriers.size ()), memoryBarriers.data (),
        static_cast<uint32_t> (bufferMemoryBarriers.size ()), bufferMemoryBarriers.data (),
        static_cast<uint32_t> (imageMemoryBarriers.size ()), imageMemoryBarriers.data ());
}
