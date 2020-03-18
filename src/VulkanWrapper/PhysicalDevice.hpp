#ifndef PHYSICALDEVICE_HPP
#define PHYSICALDEVICE_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"
#include "VulkanUtils.hpp"

#include <vulkan/vulkan.h>


class QueueFamilyProvider {
public:
    virtual uint32_t GetGraphics ()     = 0;
    virtual uint32_t GetPresentation () = 0;
    virtual uint32_t GetCompute ()      = 0;
    virtual uint32_t GetTransfer ()     = 0;
};


class QueueFamilyAcceptor {
public:
    using Acceptor = std::function<std::optional<uint32_t> (const std::vector<VkQueueFamilyProperties>&)>;

    Acceptor graphicsAcceptor;
    Acceptor presentationAcceptor;
    Acceptor computeAcceptor;
    Acceptor transferAcceptor;

    QueueFamilyAcceptor (const Acceptor& graphicsAcceptor,
                         const Acceptor& presentationAcceptor,
                         const Acceptor& computeAcceptor,
                         const Acceptor& transferAcceptor)
        : graphicsAcceptor (graphicsAcceptor)
        , presentationAcceptor (presentationAcceptor)
        , computeAcceptor (computeAcceptor)
        , transferAcceptor (transferAcceptor)
    {
    }
};

class PhysicalDevice : public Noncopyable {
private:
    VkPhysicalDevice handle;

public:
    struct QueueFamilies {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> presentation;
        std::optional<uint32_t> transfer;
        std::optional<uint32_t> compute;
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

            i++;
        }

        if (ERROR (!result.graphics.has_value () && !result.presentation.has_value ())) {
            throw std::runtime_error ("failed to find queue family indices");
        }

        return result;
    }

    static QueueFamilies FindQueueFamilyIndices (VkPhysicalDevice physicalDevice, const QueueFamilyAcceptor& acceptor)
    {
        QueueFamilies result;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies (queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, queueFamilies.data ());


        result.graphics     = acceptor.graphicsAcceptor (queueFamilies);
        result.presentation = acceptor.presentationAcceptor (queueFamilies);
        result.compute      = acceptor.computeAcceptor (queueFamilies);
        result.transfer     = acceptor.transferAcceptor (queueFamilies);

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

            std::cout << "device" << i << " api version: " << GetVersionString (deviceProperties.apiVersion) << std::endl;
            std::cout << "device" << i << " driver version: " << GetVersionString (deviceProperties.driverVersion) << std::endl;

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

    PhysicalDevice (VkInstance instance, const std::set<std::string>& requestedDeviceExtensionSet, const QueueFamilyAcceptor& acceptor)
        : handle (CreatePhysicalDevice (instance, requestedDeviceExtensionSet))
        , queueFamilies (FindQueueFamilyIndices (handle, acceptor))
    {
    }

    ~PhysicalDevice ()
    {
        handle = VK_NULL_HANDLE;
    }

    operator VkPhysicalDevice () const
    {
        return handle;
    }
};

#endif