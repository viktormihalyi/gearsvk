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

    std::set<std::string> requiredExtensionSet;
    std::transform (extensions.begin (), extensions.end (), std::inserter (requiredExtensionSet, requiredExtensionSet.end ()), [] (const char* ext) -> std::string {
        return std::string (ext);
    });

    // supported extensions
    std::set<std::string> supportedExtensionSet;
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> supportedExtensions (extensionCount);
        vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, supportedExtensions.data ());

        std::transform (supportedExtensions.begin (), supportedExtensions.end (), std::inserter (supportedExtensionSet, supportedExtensionSet.end ()), [] (const VkExtensionProperties& extProps) -> std::string {
            return extProps.extensionName;
        });
    }

    // check if the required extensions are supported
    {
        std::set<std::string> unsupportedExtensionSet;
        std::set_difference (
            requiredExtensionSet.begin (), requiredExtensionSet.end (),
            supportedExtensionSet.begin (), supportedExtensionSet.end (),
            std::inserter (unsupportedExtensionSet, unsupportedExtensionSet.end ()));

        if (ERROR (!unsupportedExtensionSet.empty ())) {
            return EXIT_FAILURE;
        }
    }

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation",
    };

    std::set<std::string> requiredValidationLayerSet;
    std::transform (validationLayers.begin (), validationLayers.end (), std::inserter (requiredValidationLayerSet, requiredValidationLayerSet.end ()), [] (const char* layer) -> std::string {
        return std::string (layer);
    });

    // supported validation layers
    std::set<std::string> supportedValidationLayerSet;
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties (&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers (layerCount);
        vkEnumerateInstanceLayerProperties (&layerCount, availableLayers.data ());
        std::transform (availableLayers.begin (), availableLayers.end (), std::inserter (supportedValidationLayerSet, supportedValidationLayerSet.end ()), [] (const VkLayerProperties& layerProps) -> std::string {
            return layerProps.layerName;
        });
    }

    // check if the required validation layers are supported
    {
        std::set<std::string> unsupportedValidationLayerSet;
        std::set_difference (
            requiredValidationLayerSet.begin (), requiredValidationLayerSet.end (),
            supportedValidationLayerSet.begin (), supportedValidationLayerSet.end (),
            std::inserter (unsupportedValidationLayerSet, unsupportedValidationLayerSet.end ()));

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

    // physical device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices (instance, &deviceCount, nullptr);
        if (ERROR (deviceCount == 0)) {
            return EXIT_FAILURE;
        }

        std::vector<VkPhysicalDevice> devices (deviceCount);
        vkEnumeratePhysicalDevices (instance, &deviceCount, devices.data ());
        ASSERT (devices.size () == 1);
        physicalDevice = devices[0];

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties (physicalDevice, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures (physicalDevice, &deviceFeatures);
    }

    // queue families
    std::optional<uint32_t> queueFamilyGraphicsIndex;
    std::optional<uint32_t> queueFamilyPresentationIndex;
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies (queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties (physicalDevice, &queueFamilyCount, queueFamilies.data ());


        int i = 0;
        for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamilyGraphicsIndex = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR (physicalDevice, i, surface, &presentSupport);
            if (presentSupport) {
                queueFamilyPresentationIndex = i;
            }

            i++;
        }

        if (ERROR (!queueFamilyGraphicsIndex.has_value () || !queueFamilyPresentationIndex.has_value ())) {
            return EXIT_FAILURE;
        }
    }

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkDevice device = VK_NULL_HANDLE;
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex        = *queueFamilyGraphicsIndex;
        queueCreateInfo.queueCount              = 1;
        float queuePriority                     = 1.0f;
        queueCreateInfo.pQueuePriorities        = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo      = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos       = &queueCreateInfo;
        createInfo.queueCreateInfoCount    = 1;
        createInfo.pEnabledFeatures        = &deviceFeatures;
        createInfo.enabledExtensionCount   = static_cast<uint32_t> (deviceExtensions.size ());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data ();

        constexpr bool enableValidationLayers = true;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount   = static_cast<uint32_t> (validationLayers.size ());
            createInfo.ppEnabledLayerNames = validationLayers.data ();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        vkCreateDevice (physicalDevice, &createInfo, nullptr, &device);
    }

    VkQueue graphicsQueue;
    vkGetDeviceQueue (device, *queueFamilyGraphicsIndex, 0, &graphicsQueue);
    VkQueue presentQueue;
    vkGetDeviceQueue (device, *queueFamilyPresentationIndex, 0, &presentQueue);


    vkDestroyDevice (device, nullptr);

    vkDestroySurfaceKHR (instance, surface, nullptr);

    DestroyDebugUtilsMessengerEXT (instance, debugMessenger, nullptr);

    vkDestroyInstance (instance, nullptr);

    return EXIT_SUCCESS;
}