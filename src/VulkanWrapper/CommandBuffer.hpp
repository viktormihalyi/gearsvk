#ifndef COMMANDBUFFER_HPP
#define COMMANDBUFFER_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

class CommandBuffer : public Noncopyable {
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
            throw std::runtime_error ("begin failed");
        }
    }


    void End () const
    {
        if (ERROR (vkEndCommandBuffer (handle) != VK_SUCCESS)) {
            throw std::runtime_error ("end failed");
        }
    }

    operator VkCommandBuffer () const
    {
        return handle;
    }
};

#endif