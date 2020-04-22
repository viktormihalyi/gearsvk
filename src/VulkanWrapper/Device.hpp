#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>


class Device : public Noncopyable {
private:
    const VkPhysicalDevice physicalDevice;
    VkDevice               handle;

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
    USING_PTR (Device);

    Device (VkPhysicalDevice physicalDevice, std::vector<uint32_t> queueFamilyIndices, std::vector<const char*> requestedDeviceExtensions)
        : physicalDevice (physicalDevice)
        , handle (VK_NULL_HANDLE)
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        for (uint32_t queueFamilyIndex : queueFamilyIndices) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex        = queueFamilyIndex;
            queueCreateInfo.queueCount              = 1;
            float queuePriority                     = 1.0f;
            queueCreateInfo.pQueuePriorities        = &queuePriority;
            queueCreateInfos.push_back (queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo      = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount    = static_cast<uint32_t> (queueCreateInfos.size ());
        createInfo.pQueueCreateInfos       = queueCreateInfos.data ();
        createInfo.pEnabledFeatures        = &deviceFeatures;
        createInfo.enabledExtensionCount   = static_cast<uint32_t> (requestedDeviceExtensions.size ());
        createInfo.ppEnabledExtensionNames = requestedDeviceExtensions.data ();
        createInfo.enabledLayerCount       = 0;

        if (ERROR (vkCreateDevice (physicalDevice, &createInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create device");
        }
    }

    ~Device ()
    {
        vkDeviceWaitIdle (handle);
        vkDestroyDevice (handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkDevice () const
    {
        return handle;
    }

    struct AllocateInfo {
        uint32_t size;
        uint32_t memoryTypeIndex;
    };

    AllocateInfo GetImageAllocateInfo (VkImage image, VkMemoryPropertyFlags propertyFlags) const
    {
        VkMemoryRequirements memRequirements = {};
        vkGetImageMemoryRequirements (handle, image, &memRequirements);
        return {static_cast<uint32_t> (memRequirements.size), FindMemoryType (memRequirements.memoryTypeBits, propertyFlags)};
    }

    AllocateInfo GetBufferAllocateInfo (VkBuffer buffer, VkMemoryPropertyFlags propertyFlags) const
    {
        VkMemoryRequirements memRequirements = {};
        vkGetBufferMemoryRequirements (handle, buffer, &memRequirements);
        return {static_cast<uint32_t> (memRequirements.size), FindMemoryType (memRequirements.memoryTypeBits, propertyFlags)};
    }

    void Wait () const
    {
        vkDeviceWaitIdle (handle);
    }
};

#endif