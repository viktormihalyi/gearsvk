#ifndef DEVICEMEMORY_HPP
#define DEVICEMEMORY_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Device.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

#include <iostream>

USING_PTR (DeviceMemory);
class GEARSVK_API DeviceMemory : public VulkanObject {
public:
    static constexpr VkMemoryPropertyFlags GPU = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    static constexpr VkMemoryPropertyFlags CPU = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

private:
    const VkDevice device;
    const size_t   allocationSize;
    const uint32_t memoryTypeIndex;
    VkDeviceMemory handle;

public:
    USING_CREATE (DeviceMemory);

    DeviceMemory (VkDevice device, const size_t allocationSize, const uint32_t memoryTypeIndex);

    virtual ~DeviceMemory () override;

    operator VkDeviceMemory () const { return handle; }

    size_t GetSize () const { return allocationSize; }
};

#endif