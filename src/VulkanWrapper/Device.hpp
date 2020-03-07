#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>


class Device : public Noncopyable {
private:
    VkDevice handle;

    static VkDevice CreateLogicalDevice (VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, const std::vector<const char*>& requestedDeviceExtensions)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex        = queueFamilyIndex;
        queueCreateInfo.queueCount              = 1;
        float queuePriority                     = 1.0f;
        queueCreateInfo.pQueuePriorities        = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo      = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount    = 1;
        createInfo.pQueueCreateInfos       = &queueCreateInfo;
        createInfo.pEnabledFeatures        = &deviceFeatures;
        createInfo.enabledExtensionCount   = static_cast<uint32_t> (requestedDeviceExtensions.size ());
        createInfo.ppEnabledExtensionNames = requestedDeviceExtensions.data ();
        createInfo.enabledLayerCount       = 0;

        VkDevice device = VK_NULL_HANDLE;
        vkCreateDevice (physicalDevice, &createInfo, nullptr, &device);
        return device;
    }

public:
    USING_PTR (Device);

    Device (VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, const std::vector<const char*>& requestedDeviceExtensions)
        : handle (CreateLogicalDevice (physicalDevice, queueFamilyIndex, requestedDeviceExtensions))
    {
    }


    ~Device ()
    {
        vkDestroyDevice (handle, nullptr);
    }

    operator VkDevice () const
    {
        return handle;
    }
};

#endif