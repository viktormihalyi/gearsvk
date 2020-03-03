#ifndef DEBUGUTILSMESSENGER_HPP
#define DEBUGUTILSMESSENGER_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

class DebugUtilsMessenger : public Noncopyable {
private:
    const VkInstance         instance;
    VkDebugUtilsMessengerEXT handle;


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

public:
    DebugUtilsMessenger (VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT callback)
        : instance (instance)
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType                              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity                    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType                        = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback                    = callback;
        createInfo.pUserData                          = nullptr;

        VkResult result = CreateDebugUtilsMessengerEXT (instance, &createInfo, nullptr, &handle);
        if (result != VK_SUCCESS) {
            throw std::runtime_error ("failed to create debug utils messenger");
        }
    }

    ~DebugUtilsMessenger ()
    {
        DestroyDebugUtilsMessengerEXT (instance, handle, nullptr);
    }

    operator VkDebugUtilsMessengerEXT () const
    {
        return handle;
    }
};

#endif