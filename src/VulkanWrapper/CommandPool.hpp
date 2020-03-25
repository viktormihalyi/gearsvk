#ifndef COMMANDPOOL_HPP
#define COMMANDPOOL_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

class CommandPool : public Noncopyable {
private:
    const VkDevice device;
    VkCommandPool  handle;

public:
    USING_PTR (CommandPool);

    CommandPool (VkDevice device, uint32_t queueIndex)
        : device (device)
        , handle (VK_NULL_HANDLE)
    {
        VkCommandPoolCreateInfo commandPoolInfo = {};
        commandPoolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex        = queueIndex;
        commandPoolInfo.flags                   = 0;
        if (ERROR (vkCreateCommandPool (device, &commandPoolInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create commandpool");
        }
    }

    ~CommandPool ()
    {
        vkDestroyCommandPool (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkCommandPool () const
    {
        return handle;
    }
};

#endif