#include "DebugUtilsMessenger.hpp"


const DebugUtilsMessenger::Settings DebugUtilsMessenger::defaultSettings {
    true,
    false,
    true,
    true,

    true,
    true,
    true,
};


const DebugUtilsMessenger::Settings DebugUtilsMessenger::noPerformance {
    true,
    false,
    true,
    true,

    true,
    true,
    false,
};


static VkResult CreateDebugUtilsMessengerEXT (VkInstance                                instance,
                                              const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                              const VkAllocationCallbacks*              pAllocator,
                                              VkDebugUtilsMessengerEXT*                 pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkCreateDebugUtilsMessengerEXT");
    if (ASSERT (func != nullptr)) {
        return func (instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


static void DestroyDebugUtilsMessengerEXT (VkInstance                   instance,
                                           VkDebugUtilsMessengerEXT     debugMessenger,
                                           const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkDestroyDebugUtilsMessengerEXT");
    if (ASSERT (func != nullptr)) {
        func (instance, debugMessenger, pAllocator);
    }
}


DebugUtilsMessenger::DebugUtilsMessenger (VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT callback, const Settings& settings)
    : instance (instance)
    , handle (VK_NULL_HANDLE)
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType                              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    createInfo.messageSeverity =
        ((settings.verbose) ? VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT : 0) |
        ((settings.info) ? VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT : 0) |
        ((settings.warning) ? VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT : 0) |
        ((settings.error) ? VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT : 0);

    createInfo.messageType =
        ((settings.general) ? VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT : 0) |
        ((settings.validation) ? VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT : 0) |
        ((settings.perforamnce) ? VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT : 0);

    createInfo.pfnUserCallback = callback;
    createInfo.pUserData       = nullptr;

    VkResult result = CreateDebugUtilsMessengerEXT (instance, &createInfo, nullptr, &handle);
    if (result != VK_SUCCESS) {
        throw std::runtime_error ("failed to create debug utils messenger");
    }
}

DebugUtilsMessenger::~DebugUtilsMessenger ()
{
    DestroyDebugUtilsMessengerEXT (instance, handle, nullptr);
    handle = VK_NULL_HANDLE;
}
