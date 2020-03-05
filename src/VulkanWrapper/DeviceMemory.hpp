#ifndef DEVICEMEMORY_HPP
#define DEVICEMEMORY_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"


class DeviceMemory : public Noncopyable {
private:
    const VkDevice       device;
    const VkDeviceMemory handle;

    static VkDeviceMemory CreateDeviceMemory (VkDevice device, size_t allocationSize, uint32_t memoryTypeIndex)
    {
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize       = allocationSize;
        allocInfo.memoryTypeIndex      = memoryTypeIndex;

        VkDeviceMemory result;
        if (ERROR (vkAllocateMemory (device, &allocInfo, nullptr, &result) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to allocate memory");
        }

        return result;
    }

public:
    USING_PTR (DeviceMemory);

    DeviceMemory (VkDevice device, size_t allocationSize, uint32_t memoryTypeIndex)
        : device (device)
        , handle (CreateDeviceMemory (device, allocationSize, memoryTypeIndex))
    {
    }

    ~DeviceMemory ()
    {
        vkFreeMemory (device, handle, nullptr);
    }

    operator VkDeviceMemory () const
    {
        return handle;
    }
};

#endif