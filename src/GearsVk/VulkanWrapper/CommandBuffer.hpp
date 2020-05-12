#ifndef COMMANDBUFFER_HPP
#define COMMANDBUFFER_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

class GEARSVK_API CommandBuffer : public Noncopyable {
private:
    const VkDevice      device;
    const VkCommandPool commandPool;
    VkCommandBuffer     handle;

public:
    USING_PTR (CommandBuffer);

    CommandBuffer (VkDevice device, VkCommandPool commandPool)
        : device (device)
        , commandPool (commandPool)
        , handle (VK_NULL_HANDLE)
    {
        VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
        commandBufferAllocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.commandPool                 = commandPool;
        commandBufferAllocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandBufferCount          = 1;

        if (ERROR (vkAllocateCommandBuffers (device, &commandBufferAllocInfo, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to allocate command buffer");
        }
    }

    ~CommandBuffer ()
    {
        vkFreeCommandBuffers (device, commandPool, 1, &handle);
        handle = VK_NULL_HANDLE;
    }

    void Begin () const
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = 0;       // Optional
        beginInfo.pInheritanceInfo         = nullptr; // Optional

        if (ERROR (vkBeginCommandBuffer (handle, &beginInfo) != VK_SUCCESS)) {
            throw std::runtime_error ("commandbuffer begin failed");
        }
    }


    void End () const
    {
        if (ERROR (vkEndCommandBuffer (handle) != VK_SUCCESS)) {
            throw std::runtime_error ("commandbuffer end failed");
        }
    }

    void Reset (bool releaseResources = true) const
    {
        if (ERROR (vkResetCommandBuffer (handle, (releaseResources) ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0) != VK_SUCCESS)) {
            throw std::runtime_error ("commandbuffer reset failed");
        }
    }

    void CmdPipelineBarrier (VkPipelineStageFlags               srcStageMask,
                             VkPipelineStageFlags               dstStageMask,
                             std::vector<VkMemoryBarrier>       memoryBarriers       = {},
                             std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers = {},
                             std::vector<VkImageMemoryBarrier>  imageMemoryBarriers  = {})
    {
        vkCmdPipelineBarrier (
            handle,
            srcStageMask,
            dstStageMask,
            0, // TODO
            static_cast<uint32_t> (memoryBarriers.size ()), memoryBarriers.data (),
            static_cast<uint32_t> (bufferMemoryBarriers.size ()), bufferMemoryBarriers.data (),
            static_cast<uint32_t> (imageMemoryBarriers.size ()), imageMemoryBarriers.data ());
    }

    operator VkCommandBuffer () const
    {
        return handle;
    }
};

#endif