#ifndef COMMANDBUFFER_HPP
#define COMMANDBUFFER_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

class CommandBuffer : public Noncopyable {
private:
    const VkDevice        device;
    const VkCommandPool   commandPool;
    const VkCommandBuffer handle;

    static VkCommandBuffer CreateCommandBuffer (VkDevice device, VkCommandPool commandPool)
    {
        VkCommandBuffer             handle;
        VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
        commandBufferAllocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.commandPool                 = commandPool;
        commandBufferAllocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandBufferCount          = 1;

        if (ERROR (vkAllocateCommandBuffers (device, &commandBufferAllocInfo, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to allocate command buffer");
        }
        return handle;
    }

public:
    USING_PTR (CommandBuffer);

    CommandBuffer (VkDevice device, VkCommandPool commandPool)
        : device (device)
        , commandPool (commandPool)
        , handle (CreateCommandBuffer (device, commandPool))
    {
    }

    ~CommandBuffer ()
    {
        vkFreeCommandBuffers (device, commandPool, 1, &handle);
    }

    operator VkCommandBuffer () const
    {
        return handle;
    }
};

#endif