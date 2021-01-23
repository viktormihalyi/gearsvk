#ifndef DEBUGUTILSMESSENGER_HPP
#define DEBUGUTILSMESSENGER_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"


USING_PTR (DebugUtilsMessenger);
class GVK_RENDERER_API DebugUtilsMessenger : public VulkanObject {
    USING_CREATE (DebugUtilsMessenger);

public:
    using Callback = std::function<void (VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                         VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)>;

    struct Settings {
        bool verbose;
        bool info;
        bool warning;
        bool error;

        bool general;
        bool validation;
        bool perforamnce;
    };

private:
    const VkInstance         instance;
    VkDebugUtilsMessengerEXT handle;

    Callback callback;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (
        VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT             messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void*                                       userData);

public:
    static const Settings defaultSettings;
    static const Settings noPerformance;

    DebugUtilsMessenger (VkInstance instance, const Callback& callback, const Settings& settings = defaultSettings);

    ~DebugUtilsMessenger ();

    operator VkDebugUtilsMessengerEXT () const { return handle; }
};

#endif