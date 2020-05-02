#ifndef SINGLETIMECOMMAND_HPP
#define SINGLETIMECOMMAND_HPP

#include "Assert.hpp"
#include "CommandBuffer.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>


class SingleTimeCommand final : public Noncopyable {
private:
    const VkDevice      device;
    const VkCommandPool commandPool;
    const VkQueue       queue;
    const CommandBuffer commandBuffer;

public:
    SingleTimeCommand (VkDevice device, VkCommandPool commandPool, VkQueue queue)
        : device (device)
        , queue (queue)
        , commandPool (commandPool)
        , commandBuffer (device, commandPool)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (ERROR (vkBeginCommandBuffer (commandBuffer, &beginInfo) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to begin one time commandbuffer");
        }
    }

    ~SingleTimeCommand ()
    {
        if (ERROR (vkEndCommandBuffer (commandBuffer) != VK_SUCCESS)) {
            return;
        }

        VkCommandBuffer handle = commandBuffer;

        VkSubmitInfo submitInfo       = {};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &handle;

        vkQueueSubmit (queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle (queue);
    }

    operator VkCommandBuffer () const { return commandBuffer; }
};

#endif