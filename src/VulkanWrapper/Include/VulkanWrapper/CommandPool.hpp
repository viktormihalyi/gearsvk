#ifndef COMMANDPOOL_HPP
#define COMMANDPOOL_HPP

#include <vulkan/vulkan.h>

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "VulkanObject.hpp"
#include "Utils/Noncopyable.hpp"

#include <stdexcept>

namespace GVK {

class /* VULKANWRAPPER_API */ CommandPool : public VulkanObject {
private:
    VkDevice                       device;
    GVK::MovablePtr<VkCommandPool> handle;

public:
    CommandPool (VkDevice device, uint32_t queueIndex)
        : device (device)
        , handle (VK_NULL_HANDLE)
    {
        VkCommandPoolCreateInfo commandPoolInfo = {};
        commandPoolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex        = queueIndex;
        commandPoolInfo.flags                   = 0;
        if (GVK_ERROR (vkCreateCommandPool (device, &commandPoolInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create commandpool");
        }
    }

    CommandPool (CommandPool&&) = default;
    CommandPool& operator= (CommandPool&&) = default;

    ~CommandPool ()
    {
        vkDestroyCommandPool (device, handle, nullptr);
        handle = nullptr;
    }

    operator VkCommandPool () const
    {
        return handle;
    }
};

} // namespace GVK

#endif