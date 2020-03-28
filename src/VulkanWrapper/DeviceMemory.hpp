#ifndef DEVICEMEMORY_HPP
#define DEVICEMEMORY_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

#include <iostream>

class DeviceMemory : public Noncopyable {
public:
    static constexpr VkMemoryPropertyFlags GPU = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    static constexpr VkMemoryPropertyFlags CPU = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

private:
    const VkDevice device;
    VkDeviceMemory handle;

public:
    USING_PTR (DeviceMemory);

    DeviceMemory (VkDevice device, size_t allocationSize, uint32_t memoryTypeIndex)
        : device (device)
        , handle (VK_NULL_HANDLE)
    {
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize       = allocationSize;
        allocInfo.memoryTypeIndex      = memoryTypeIndex;

        if (ERROR (vkAllocateMemory (device, &allocInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to allocate memory");
        }

        std::cout << "allocated " << allocationSize << " bytes (idx: " << memoryTypeIndex << ")" << std::endl;
    }

    DeviceMemory (VkDevice device, Device::AllocateInfo allocateInfo)
        : DeviceMemory (device, allocateInfo.size, allocateInfo.memoryTypeIndex)
    {
    }

    ~DeviceMemory ()
    {
        vkFreeMemory (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkDeviceMemory () const
    {
        return handle;
    }
};

#endif