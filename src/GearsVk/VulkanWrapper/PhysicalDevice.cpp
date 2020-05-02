#include "PhysicalDevice.hpp"


DefaultQueueFamilySelector defaultQueueFamilySelector;


static std::optional<uint32_t> AcceptFirstGraphicsAndPresentSupport (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<VkQueueFamilyProperties>& queueFamilies)
{
    if (surface == VK_NULL_HANDLE) {
        return std::nullopt;
    }

    uint32_t i = 0;
    for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR (physicalDevice, i, surface, &presentSupport);
        if (presentSupport && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            return i;
        }

        i++;
    }

    return std::nullopt;
}


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


DefaultQueueFamilySelector::DefaultQueueFamilySelector ()
    : QueueFamilySelector (
          AcceptFirstWithFlag (VK_QUEUE_GRAPHICS_BIT),
          AcceptFirstPresentSupport,
          AcceptFirstWithFlag (VK_QUEUE_COMPUTE_BIT),
          AcceptFirstWithFlag (VK_QUEUE_TRANSFER_BIT))
{
}


static PhysicalDevice::QueueFamilies FindQueueFamilyIndices (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const QueueFamilySelector& Selector)
{
    PhysicalDevice::QueueFamilies result;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies (queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, queueFamilies.data ());

    result.graphics     = Selector.graphicsSelector (physicalDevice, surface, queueFamilies);
    result.presentation = Selector.presentationSelector (physicalDevice, surface, queueFamilies);
    result.compute      = Selector.computeSelector (physicalDevice, surface, queueFamilies);
    result.transfer     = Selector.transferSelector (physicalDevice, surface, queueFamilies);

    if (result.presentation) {
        ASSERT (result.graphics == result.presentation); // TODO handle different queue indices ...
    }

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


PhysicalDevice::PhysicalDevice (VkInstance instance, VkSurfaceKHR surface, const std::set<std::string>& requestedDeviceExtensionSet, const QueueFamilySelector& selector)
    : instance (instance)
    , requestedDeviceExtensionSet (requestedDeviceExtensionSet)
    , selector (selector)
{
    RecreateForSurface (surface);

    VkFormatProperties prop = {};

    vkGetPhysicalDeviceFormatProperties (handle, VK_FORMAT_R8_UINT, &prop);
    vkGetPhysicalDeviceFormatProperties (handle, VK_FORMAT_R8G8_UINT, &prop);
    vkGetPhysicalDeviceFormatProperties (handle, VK_FORMAT_R8G8B8_UINT, &prop);
    vkGetPhysicalDeviceFormatProperties (handle, VK_FORMAT_R8G8B8A8_UINT, &prop);

    vkGetPhysicalDeviceFormatProperties (handle, VK_FORMAT_R8_SRGB, &prop);
    vkGetPhysicalDeviceFormatProperties (handle, VK_FORMAT_R8G8_SRGB, &prop);
    vkGetPhysicalDeviceFormatProperties (handle, VK_FORMAT_R8G8B8_SRGB, &prop);
    vkGetPhysicalDeviceFormatProperties (handle, VK_FORMAT_R8G8B8A8_SRGB, &prop);

    vkGetPhysicalDeviceFormatProperties (handle, VK_FORMAT_R8_UNORM, &prop);
    vkGetPhysicalDeviceFormatProperties (handle, VK_FORMAT_R8G8_UNORM, &prop);
    vkGetPhysicalDeviceFormatProperties (handle, VK_FORMAT_R8G8B8_UNORM, &prop);
    vkGetPhysicalDeviceFormatProperties (handle, VK_FORMAT_R8G8B8A8_UNORM, &prop);

}


PhysicalDevice::~PhysicalDevice ()
{
    handle = VK_NULL_HANDLE;
}


void PhysicalDevice::RecreateForSurface (VkSurfaceKHR surface)
{
    handle        = CreatePhysicalDevice (instance, requestedDeviceExtensionSet);
    queueFamilies = FindQueueFamilyIndices (handle, surface, selector);
}
