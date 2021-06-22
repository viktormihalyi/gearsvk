#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "Utils/Utils.hpp"
#include "VulkanObject.hpp"

#include <vulkan/vulkan.h>

namespace GVK {

class VULKANWRAPPER_API Device : public Noncopyable {
public:
    virtual ~Device () = default;

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