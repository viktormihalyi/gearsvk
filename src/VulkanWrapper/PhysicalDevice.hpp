#ifndef PHYSICALDEVICE_HPP
#define PHYSICALDEVICE_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"
#include "VulkanUtils.hpp"

#include <vulkan/vulkan.h>


class QueueFamilySelector {
public:
    using Selector = std::function<std::optional<uint32_t> (VkPhysicalDevice, VkSurfaceKHR, const std::vector<VkQueueFamilyProperties>&)>;

public:
    Selector graphicsSelector;
    Selector presentationSelector;
    Selector computeSelector;
    Selector transferSelector;

    QueueFamilySelector (const Selector& graphicsSelector,
                         const Selector& presentationSelector,
                         const Selector& computeSelector,
                         const Selector& transferSelector)
        : graphicsSelector (graphicsSelector)
        , presentationSelector (presentationSelector)
        , computeSelector (computeSelector)
        , transferSelector (transferSelector)
    {
    }

    virtual ~QueueFamilySelector () {}
};


class DefaultQueueFamilySelector final : public QueueFamilySelector {
private:
    static std::optional<uint32_t> AcceptFirstPresentSupport (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<VkQueueFamilyProperties>& queueFamilies)
    {
        if (surface == VK_NULL_HANDLE) {
            return std::nullopt;
        }

        uint32_t i = 0;
        for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR (physicalDevice, i, surface, &presentSupport);
            if (presentSupport) {
                return i;
            }

            i++;
        }

        return std::nullopt;
    }


    static auto AcceptFirstWithFlag (VkQueueFlagBits flagbits)
    {
        return [=] (VkPhysicalDevice, VkSurfaceKHR, const std::vector<VkQueueFamilyProperties>& props) -> std::optional<uint32_t> {
            uint32_t i = 0;
            for (const auto& p : props) {
                if (p.queueFlags & flagbits) {
                    return i;
                }
                ++i;
            }
            return std::nullopt;
        };
    }

public:
    DefaultQueueFamilySelector ()
        : QueueFamilySelector (
              AcceptFirstWithFlag (VK_QUEUE_GRAPHICS_BIT),
              AcceptFirstPresentSupport,
              AcceptFirstWithFlag (VK_QUEUE_COMPUTE_BIT),
              AcceptFirstWithFlag (VK_QUEUE_TRANSFER_BIT))
    {
    }
};


static DefaultQueueFamilySelector defaultQueueFamilySelector;


class PhysicalDevice : public Noncopyable {
public:
    struct QueueFamilies {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> presentation;
        std::optional<uint32_t> transfer;
        std::optional<uint32_t> compute;
    };

private:
    VkPhysicalDevice handle;
    QueueFamilies    queueFamilies;

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

            i++;
        }

        if (ERROR (!result.graphics.has_value () && !result.presentation.has_value ())) {
            throw std::runtime_error ("failed to find queue family indices");
        }

        return result;
    }

    static QueueFamilies FindQueueFamilyIndices (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const QueueFamilySelector& Selector)
    {
        QueueFamilies result;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies (queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, queueFamilies.data ());

        result.graphics     = Selector.graphicsSelector (physicalDevice, surface, queueFamilies);
        result.presentation = Selector.presentationSelector (physicalDevice, surface, queueFamilies);
        result.compute      = Selector.computeSelector (physicalDevice, surface, queueFamilies);
        result.transfer     = Selector.transferSelector (physicalDevice, surface, queueFamilies);

        return result;
    }

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
    USING_PTR (PhysicalDevice);

    PhysicalDevice (VkInstance instance, VkSurfaceKHR surface, const std::set<std::string>& requestedDeviceExtensionSet, const QueueFamilySelector& Selector = defaultQueueFamilySelector)
        : handle (CreatePhysicalDevice (instance, requestedDeviceExtensionSet))
        , queueFamilies (FindQueueFamilyIndices (handle, surface, Selector))
    {
    }

    ~PhysicalDevice ()
    {
        handle = VK_NULL_HANDLE;
    }

    operator VkPhysicalDevice () const { return handle; }

    QueueFamilies GetQueueFamilies () const { return queueFamilies; }
};

#endif