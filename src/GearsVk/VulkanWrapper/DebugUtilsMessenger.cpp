#include "DebugUtilsMessenger.hpp"

namespace GVK {

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
    if (GVK_VERIFY (func)) {
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
    if (GVK_VERIFY (func != nullptr)) {
        func (instance, debugMessenger, pAllocator);
    }
}


VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessenger::debugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                                   VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                                   const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                                   void*                                       userData)
{
    if (GVK_VERIFY (userData != nullptr)) {
        reinterpret_cast<DebugUtilsMessenger*> (userData)->callback (messageSeverity, messageType, callbackData);
    }

    return VK_FALSE;
}


DebugUtilsMessenger::DebugUtilsMessenger (VkInstance instance, const Callback& callback, const Settings& settings)
    : instance (instance)
    , handle (VK_NULL_HANDLE)
    , callback (callback)
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

    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData       = this;

    VkResult result = CreateDebugUtilsMessengerEXT (instance, &createInfo, nullptr, &handle);
    if (GVK_ERROR (result != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create debug utils messenger");
    }
}


DebugUtilsMessenger::~DebugUtilsMessenger ()
{
    DestroyDebugUtilsMessengerEXT (instance, handle, nullptr);
    handle = VK_NULL_HANDLE;
}

}