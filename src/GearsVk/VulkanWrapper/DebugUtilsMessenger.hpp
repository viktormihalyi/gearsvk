#ifndef DEBUGUTILSMESSENGER_HPP
#define DEBUGUTILSMESSENGER_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

USING_PTR (DebugUtilsMessenger);

class GEARSVK_API DebugUtilsMessenger : public Noncopyable {
private:
    const VkInstance         instance;
    VkDebugUtilsMessengerEXT handle;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (
        VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT             messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void*                                       userData);

public:
    using Callback = std::function<void (VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                         VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)>;

public:
    struct Settings {
        bool verbose;
        bool info;
        bool warning;
        bool error;

        bool general;
        bool validation;
        bool perforamnce;
    };

    static const Settings defaultSettings;
    static const Settings noPerformance;

    Callback callback;

    USING_CREATE (DebugUtilsMessenger);

    DebugUtilsMessenger (VkInstance instance, const Callback& callback, const Settings& settings = defaultSettings);

    ~DebugUtilsMessenger ();

    operator VkDebugUtilsMessengerEXT () const { return handle; }
};

#endif