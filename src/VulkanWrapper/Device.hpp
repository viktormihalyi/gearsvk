#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>


class Device : public Noncopyable {
private:
    VkDevice handle;

    static VkDevice CreateLogicalDevice (VkPhysicalDevice physicalDevice, uint32_t graphicsQueueFamilyIndex, const std::vector<const char*>& requestedDeviceExtensions)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex        = graphicsQueueFamilyIndex;
        queueCreateInfo.queueCount              = 1;
        float queuePriority                     = 1.0f;
        queueCreateInfo.pQueuePriorities        = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo      = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos       = &queueCreateInfo;
        createInfo.queueCreateInfoCount    = 1;
        createInfo.pEnabledFeatures        = &deviceFeatures;
        createInfo.enabledExtensionCount   = static_cast<uint32_t> (requestedDeviceExtensions.size ());
        createInfo.ppEnabledExtensionNames = requestedDeviceExtensions.data ();
        createInfo.enabledLayerCount       = 0;

        VkDevice device = VK_NULL_HANDLE;
        vkCreateDevice (physicalDevice, &createInfo, nullptr, &device);
        return device;
    }

public:
    Device (VkPhysicalDevice physicalDevice, uint32_t graphicsQueueFamilyIndex, const std::vector<const char*>& requestedDeviceExtensions)
        : handle (CreateLogicalDevice (physicalDevice, graphicsQueueFamilyIndex, requestedDeviceExtensions))
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