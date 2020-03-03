#ifndef COMMANDPOOL_HPP
#define COMMANDPOOL_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

class CommandPool : public Noncopyable {
private:
    const VkDevice      device;
    const VkCommandPool handle;

    static VkCommandPool CreateCommandPool (VkDevice device, uint32_t queueIndex)
    {
        VkCommandPool handle;

        VkCommandPoolCreateInfo commandPoolInfo = {};
        commandPoolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex        = queueIndex;
        commandPoolInfo.flags                   = 0;
        if (ERROR (vkCreateCommandPool (device, &commandPoolInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create commandpool");
        }

        return handle;
    }

public:
    CommandPool (VkDevice device, uint32_t queueIndex)
        : device (device)
        , handle (CreateCommandPool (device, queueIndex))
    {
    }

    ~CommandPool ()
    {
        vkDestroyCommandPool (device, handle, nullptr);
    }

    operator VkCommandPool () const
    {
        return handle;
    }
};

#endif