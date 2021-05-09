#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "Assert.hpp"
#include "MovablePtr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

#include <vulkan/vulkan.h>

namespace GVK {

class GVK_RENDERER_API Device {
public:
    virtual ~Device () = default;

    virtual      operator VkDevice () const = 0;
    virtual void Wait () const              = 0;
};


class GVK_RENDERER_API DeviceObject : public VulkanObject, public Device {
private:
    VkPhysicalDevice          physicalDevice;
    GVK::MovablePtr<VkDevice> handle;

    uint32_t FindMemoryType (uint32_t typeFilter, VkMemoryPropertyFlags properties) const
    {
        VkPhysicalDeviceMemoryProperties memProperties = {};
        vkGetPhysicalDeviceMemoryProperties (physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error ("failed to find suitable memory type!");
    }


public:
    DeviceObject (VkPhysicalDevice physicalDevice, std::vector<uint32_t> queueFamilyIndices, std::vector<const char*> requestedDeviceExtensions)
        : physicalDevice (physicalDevice)
        , handle (VK_NULL_HANDLE)
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        for (uint32_t queueFamilyIndex : queueFamilyIndices) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex        = queueFamilyIndex;
            queueCreateInfo.queueCount              = 1;
            const float queuePriority               = 1.0f;
            queueCreateInfo.pQueuePriorities        = &queuePriority;
            queueCreateInfos.push_back (queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.shaderInt64 = VK_TRUE;

        VkDeviceCreateInfo createInfo      = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount    = static_cast<uint32_t> (queueCreateInfos.size ());
        createInfo.pQueueCreateInfos       = queueCreateInfos.data ();
        createInfo.pEnabledFeatures        = &deviceFeatures;
        createInfo.enabledExtensionCount   = static_cast<uint32_t> (requestedDeviceExtensions.size ());
        createInfo.ppEnabledExtensionNames = requestedDeviceExtensions.data ();
        createInfo.enabledLayerCount       = 0;

        if (GVK_ERROR (vkCreateDevice (physicalDevice, &createInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create device");
        }
    }

    DeviceObject (DeviceObject&&) = default;
    DeviceObject& operator= (DeviceObject&&) = default;

    virtual ~DeviceObject () override
    {
        vkDeviceWaitIdle (handle);
        vkDestroyDevice (handle, nullptr);
        handle = nullptr;
    }

    virtual operator VkDevice () const override
    {
        return handle;
    }

    virtual void Wait () const override
    {
        vkDeviceWaitIdle (handle);
    }
};

} // namespace GVK

#endif