#include "Assert.hpp"
#include "GLFWWindowProvider.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <vector>

#include <vulkan/vulkan.h>


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*                                       pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT (VkInstance                                instance,
                                       const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                       const VkAllocationCallbacks*              pAllocator,
                                       VkDebugUtilsMessengerEXT*                 pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func (instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


void DestroyDebugUtilsMessengerEXT (VkInstance                   instance,
                                    VkDebugUtilsMessengerEXT     debugMessenger,
                                    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func (instance, debugMessenger, pAllocator);
    }
}

#include <functional>

template<typename SourceType, typename DestType>
static std::set<DestType> ToSet (const std::vector<SourceType>& vec)
{
    std::set<DestType> result;
    std::transform (std::begin (vec), std::end (vec), std::inserter (result, std::end (result)), [] (const SourceType& x) {
        return DestType (x);
    });
    return result;
}

template<typename SourceType, typename DestType>
static std::set<DestType> ToSet (const std::vector<SourceType>& vec, const std::function<DestType (const SourceType&)>& converter)
{
    std::set<DestType> result;
    std::transform (std::begin (vec), std::end (vec), std::inserter (result, std::end (result)), converter);
    return result;
}

template<typename T>
static std::set<T> SetDiff (const std::set<T>& left, const std::set<T>& right)
{
    std::set<T> diff;
    std::set_difference (std::begin (left), std::end (left), std::begin (right), std::end (right), std::inserter (diff, std::end (diff)));
    return diff;
}


struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};


VkSurfaceFormatKHR ChooseSwapSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& formats)
{
    if (ERROR (formats.empty ())) {
        return {};
    }

    for (const VkSurfaceFormatKHR& availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    ERROR (true);

    return formats[0];
}

VkPresentModeKHR ChooseSwapPresentMode (const std::vector<VkPresentModeKHR>& modes)
{
    if (ERROR (modes.empty ())) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    for (const VkPresentModeKHR& availablePresentMode : modes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    ERROR (true);

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent (const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {800, 600};
        actualExtent.width      = std::clamp (capabilities.minImageExtent.width, capabilities.maxImageExtent.width, actualExtent.width);
        actualExtent.height     = std::clamp (capabilities.minImageExtent.height, capabilities.maxImageExtent.height, actualExtent.height);
        return actualExtent;
    }
}


int main (int argc, char* argv[])
{
    std::cout << Utils::PROJECT_ROOT.u8string () << std::endl;

    WindowProviderUPtr windowProvider = std::make_unique<GLFWWindowProvider> ();

    // platform required extensionss
    std::vector<const char*> extensions;
    {
        extensions = windowProvider->GetExtensions ();
        extensions.push_back (VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    const std::set<std::string> requiredExtensionSet = ToSet<const char*, std::string> (extensions);

    auto extensionNameAccessor = [] (const VkExtensionProperties& props) { return props.extensionName; };
    auto layerNameAccessor     = [] (const VkLayerProperties& props) { return props.layerName; };


    // supported extensions
    std::set<std::string> supportedExtensionSet;
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> supportedExtensions (extensionCount);
        vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, supportedExtensions.data ());

        supportedExtensionSet = ToSet<VkExtensionProperties, std::string> (supportedExtensions, extensionNameAccessor);
    }

    // check if the required extensions are supported
    {
        const std::set<std::string> unsupportedExtensionSet = SetDiff (requiredExtensionSet, supportedExtensionSet);
        if (ERROR (!unsupportedExtensionSet.empty ())) {
            return EXIT_FAILURE;
        }
    }

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation",
    };

    std::set<std::string> requiredValidationLayerSet = ToSet<const char*, std::string> (validationLayers);

    // supported validation layers
    std::set<std::string> supportedValidationLayerSet;
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties (&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers (layerCount);
        vkEnumerateInstanceLayerProperties (&layerCount, availableLayers.data ());

        supportedValidationLayerSet = ToSet<VkLayerProperties, std::string> (availableLayers, layerNameAccessor);
    }

    // check if the required validation layers are supported
    {
        const std::set<std::string> unsupportedValidationLayerSet = SetDiff (requiredValidationLayerSet, supportedValidationLayerSet);
        if (ERROR (!unsupportedValidationLayerSet.empty ())) {
            return EXIT_FAILURE;
        }
    }


    VkInstance instance = VK_NULL_HANDLE;
    {
        VkApplicationInfo appInfo  = {};
        appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName   = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION (1, 0, 0);
        appInfo.pEngineName        = "No Engine";
        appInfo.engineVersion      = VK_MAKE_VERSION (1, 0, 0);
        appInfo.apiVersion         = VK_API_VERSION_1_1;

        VkInstanceCreateInfo createInfo    = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo        = &appInfo;
        createInfo.enabledExtensionCount   = extensions.size ();
        createInfo.ppEnabledExtensionNames = extensions.data ();
        createInfo.enabledLayerCount       = static_cast<uint32_t> (validationLayers.size ());
        createInfo.ppEnabledLayerNames     = validationLayers.data ();

        VkResult result = vkCreateInstance (&createInfo, nullptr, &instance);
        if (result != VK_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    VkDebugUtilsMessengerEXT debugMessenger;
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType                              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity                    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType                        = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback                    = debugCallback;
        createInfo.pUserData                          = nullptr;

        VkResult result = CreateDebugUtilsMessengerEXT (instance, &createInfo, nullptr, &debugMessenger);
        if (result != VK_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    VkSurfaceKHR surface = windowProvider->CreateSurface (instance);
    if (ERROR (surface == VK_NULL_HANDLE)) {
        return EXIT_FAILURE;
    }

    const std::vector<const char*> requestedDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    const std::set<std::string> requestedDeviceExtensionSet = ToSet<const char*, std::string> (requestedDeviceExtensions);

    // physical device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices (instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices (deviceCount);
        vkEnumeratePhysicalDevices (instance, &deviceCount, devices.data ());

        std::optional<uint32_t> physicalDeviceIndex;

        uint32_t i = 0;
        for (VkPhysicalDevice device : devices) {
            uint32_t deviceExtensionCount = 0;
            vkEnumerateDeviceExtensionProperties (device, nullptr, &deviceExtensionCount, nullptr);
            std::vector<VkExtensionProperties> supportedDeviceExtensions (deviceExtensionCount);
            vkEnumerateDeviceExtensionProperties (device, nullptr, &deviceExtensionCount, supportedDeviceExtensions.data ());

            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties (device, &deviceProperties);

            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures (device, &deviceFeatures);

            const std::set<std::string> supportedDeviceExtensionSet   = ToSet<VkExtensionProperties, std::string> (supportedDeviceExtensions, extensionNameAccessor);
            const std::set<std::string> unsupportedDeviceExtensionSet = SetDiff (requestedDeviceExtensionSet, supportedDeviceExtensionSet);

            if (unsupportedDeviceExtensionSet.empty ()) {
                physicalDeviceIndex = i;
            }

            ++i;
        }

        if (ERROR (!physicalDeviceIndex.has_value ())) {
            return EXIT_FAILURE;
        }

        physicalDevice = devices[*physicalDeviceIndex];
    }

    struct QueueFamilies {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> presentation;
    } queueFamilyIndices;

    // queue families
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies (queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, queueFamilies.data ());


        int i = 0;
        for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamilyIndices.graphics = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR (physicalDevice, i, surface, &presentSupport);
            if (presentSupport) {
                queueFamilyIndices.presentation = i;
            }

            i++;
        }

        if (ERROR (!queueFamilyIndices.graphics.has_value () || !queueFamilyIndices.presentation.has_value ())) {
            return EXIT_FAILURE;
        }
    }


    VkDevice device = VK_NULL_HANDLE;
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex        = *queueFamilyIndices.graphics;
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

        constexpr bool enableValidationLayers = true;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount   = static_cast<uint32_t> (validationLayers.size ());
            createInfo.ppEnabledLayerNames = validationLayers.data ();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        vkCreateDevice (physicalDevice, &createInfo, nullptr, &device);
    }


    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR (physicalDevice, surface, &details.capabilities);
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR (physicalDevice, surface, &formatCount, nullptr);
    details.formats.resize (formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR (physicalDevice, surface, &formatCount, details.formats.data ());
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR (physicalDevice, surface, &presentModeCount, nullptr);
    details.presentModes.resize (presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR (physicalDevice, surface, &presentModeCount, details.presentModes.data ());

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat (details.formats);
    VkPresentModeKHR   presentMode   = ChooseSwapPresentMode (details.presentModes);
    VkExtent2D         extent        = ChooseSwapExtent (details.capabilities);

    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface                  = surface;
    createInfo.minImageCount            = imageCount;
    createInfo.imageFormat              = surfaceFormat.format;
    createInfo.imageColorSpace          = surfaceFormat.colorSpace;
    createInfo.imageExtent              = extent;
    createInfo.imageArrayLayers         = 1;
    createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndicesData[] = {*queueFamilyIndices.graphics, *queueFamilyIndices.presentation};
    if (ERROR (*queueFamilyIndices.graphics != *queueFamilyIndices.presentation)) {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndicesData;
    } else {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;       // Optional
        createInfo.pQueueFamilyIndices   = nullptr; // Optional
    }
    createInfo.preTransform   = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain;
    if (ERROR (vkCreateSwapchainKHR (device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)) {
        return EXIT_FAILURE;
    }

    vkGetSwapchainImagesKHR (device, swapChain, &imageCount, nullptr);
    std::vector<VkImage> swapChainImages (imageCount);
    vkGetSwapchainImagesKHR (device, swapChain, &imageCount, swapChainImages.data ());
    std::vector<VkImageView> swapChainImageViews (imageCount);

    for (size_t i = 0; i < swapChainImages.size (); ++i) {
        VkImageViewCreateInfo createInfo           = {};
        createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image                           = swapChainImages[i];
        createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format                          = surfaceFormat.format;
        createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;
        if (ERROR (vkCreateImageView (device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)) {
            return EXIT_FAILURE;
        }
    }

    for (VkImageView imageView : swapChainImageViews) {
        vkDestroyImageView (device, imageView, nullptr);
    }

    vkDestroySwapchainKHR (device, swapChain, nullptr);

    vkDestroyDevice (device, nullptr);

    vkDestroySurfaceKHR (instance, surface, nullptr);

    DestroyDebugUtilsMessengerEXT (instance, debugMessenger, nullptr);

    vkDestroyInstance (instance, nullptr);

    return EXIT_SUCCESS;
}