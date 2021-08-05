#ifndef PHYSICALDEVICE_HPP
#define PHYSICALDEVICE_HPP

#include "VulkanWrapper/VulkanWrapperAPI.hpp"

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "Utils/Utils.hpp"
#include "Utils/Noncopyable.hpp"

#include <vulkan/vulkan.h>

#include <functional>
#include <optional>
#include <vector>

namespace GVK {

class VULKANWRAPPER_API PhysicalDevice final : public Noncopyable {
public:
    struct QueueFamilies {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> presentation;
        std::optional<uint32_t> transfer;
        std::optional<uint32_t> compute;
    };

private:
    VkInstance                        instance;
    GVK::MovablePtr<VkPhysicalDevice> handle;
    std::set<std::string>             requestedDeviceExtensionSet;
    QueueFamilies                     queueFamilies;

    std::unique_ptr<VkPhysicalDeviceProperties> properties;
    std::unique_ptr<VkPhysicalDeviceFeatures>   features;

public:
    PhysicalDevice (VkInstance                   instance,
                    VkSurfaceKHR                 surface, // VK_NULL_HANDLE is accepted
                    const std::set<std::string>& requestedDeviceExtensionSet);

    virtual ~PhysicalDevice () override;

    bool CheckSurfaceSupported (VkSurfaceKHR surface) const;

    operator VkPhysicalDevice () const { return handle; }

    QueueFamilies GetQueueFamilies () const { return queueFamilies; }

    VkPhysicalDeviceProperties GetProperties () const
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties (handle, &properties);
        return properties;
    }

    VkPhysicalDeviceFeatures GetFeatures () const
    {
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures (handle, &features);
        return features;
    }
};

} // namespace GVK

#endif