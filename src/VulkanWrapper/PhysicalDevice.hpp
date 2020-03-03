#ifndef PHYSICALDEVICE_HPP
#define PHYSICALDEVICE_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>


class PhysicalDevice : public Noncopyable {
private:
    VkPhysicalDevice handle;

public:
    struct QueueFamilies {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> presentation;

        bool IsValid () const
        {
            return graphics.has_value () && presentation.has_value ();
        }
    };

    // TODO private
    QueueFamilies queueFamilies;

private:
    static QueueFamilies CreateQueueFamilyIndices (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        QueueFamilies result;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies (queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, queueFamilies.data ());


        int i = 0;
        for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                result.graphics = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR (physicalDevice, i, surface, &presentSupport);
            if (presentSupport) {
                result.presentation = i;
            }

            if (result.IsValid ()) {
                break;
            }

            i++;
        }

        if (ERROR (!result.IsValid ())) {
            throw std::runtime_error ("failed to find queue family indices");
        }

        return result;
    }

private:
    static VkPhysicalDevice CreatePhysicalDevice (VkInstance instance, const std::set<std::string>& requestedDeviceExtensionSet)
    {
        // query physical devices
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices (instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices (deviceCount);
        vkEnumeratePhysicalDevices (instance, &deviceCount, devices.data ());

        std::optional<uint32_t> physicalDeviceIndex;

        auto extensionNameAccessor = [] (const VkExtensionProperties& props) { return props.extensionName; };

        uint32_t i = 0;
        for (VkPhysicalDevice device : devices) {
            // check supported extensions
            uint32_t deviceExtensionCount = 0;
            vkEnumerateDeviceExtensionProperties (device, nullptr, &deviceExtensionCount, nullptr);
            std::vector<VkExtensionProperties> supportedDeviceExtensions (deviceExtensionCount);
            vkEnumerateDeviceExtensionProperties (device, nullptr, &deviceExtensionCount, supportedDeviceExtensions.data ());

            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties (device, &deviceProperties);

            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures (device, &deviceFeatures);

            // check if the device supports all requested extensions
            const std::set<std::string> supportedDeviceExtensionSet   = Utils::ToSet<VkExtensionProperties, std::string> (supportedDeviceExtensions, extensionNameAccessor);
            const std::set<std::string> unsupportedDeviceExtensionSet = Utils::SetDiff (requestedDeviceExtensionSet, supportedDeviceExtensionSet);

            if (unsupportedDeviceExtensionSet.empty ()) {
                physicalDeviceIndex = i;
                break;
            }

            ++i;
        }

        if (ERROR (!physicalDeviceIndex.has_value ())) {
            throw std::runtime_error ("No physical device available");
        }

        return devices[*physicalDeviceIndex];
    }

public:
    PhysicalDevice (VkInstance instance, VkSurfaceKHR surface, const std::set<std::string>& requestedDeviceExtensionSet)
        : handle (CreatePhysicalDevice (instance, requestedDeviceExtensionSet))
        , queueFamilies (CreateQueueFamilyIndices (handle, surface))
    {
    }

    operator VkPhysicalDevice () const
    {
        return handle;
    }
};

#endif