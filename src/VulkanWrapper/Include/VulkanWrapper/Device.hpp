#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "VulkanObject.hpp"
#include "Utils/Noncopyable.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace GVK {

class VULKANWRAPPER_API Device : public Nonmovable {
public:
    Device () = default;
    
    virtual ~Device ();

    Device (const Device&) = delete;
    Device& operator= (const Device&) = delete;

    Device (Device&&) = default;
    Device& operator= (Device&&) = default;

    virtual      operator VkDevice () const = 0;
    virtual void Wait () const              = 0;
};


class VULKANWRAPPER_API DeviceObject : public VulkanObject, public Device {
private:
    VkPhysicalDevice          physicalDevice;
    GVK::MovablePtr<VkDevice> handle;

public:
    DeviceObject (VkPhysicalDevice physicalDevice, std::vector<uint32_t> queueFamilyIndices, std::vector<const char*> requestedDeviceExtensions);

    DeviceObject (DeviceObject&&) = default;
    DeviceObject& operator= (DeviceObject&&) = default;

    virtual ~DeviceObject () override;

    virtual void* GetHandleForName () const override { return handle; }

    virtual VkObjectType GetObjectTypeForName () const override { return VK_OBJECT_TYPE_DEVICE; }

    virtual operator VkDevice () const override
    {
        return handle;
    }

    virtual void Wait () const override
    {
        vkDeviceWaitIdle (handle);
    }

private:
    uint32_t FindMemoryType (uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
};

} // namespace GVK

#endif