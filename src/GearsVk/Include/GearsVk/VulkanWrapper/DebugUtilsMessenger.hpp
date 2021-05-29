#ifndef DEBUGUTILSMESSENGER_HPP
#define DEBUGUTILSMESSENGER_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "MovablePtr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"
#include "Noncopyable.hpp"

namespace GVK {

class GVK_RENDERER_API DebugUtilsMessenger : public VulkanObject, public Noncopyable {
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
    VkInstance                                instance;
    GVK::MovablePtr<VkDebugUtilsMessengerEXT> handle;
    Callback                                  callback;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (
        VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT             messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void*                                       userData);

public:
    static const Settings defaultSettings;
    static const Settings noPerformance;

    DebugUtilsMessenger (VkInstance instance, const Callback& callback, const Settings& settings = defaultSettings);

    DebugUtilsMessenger (DebugUtilsMessenger&&) = default;
    DebugUtilsMessenger& operator= (DebugUtilsMessenger&&) = default;

    virtual ~DebugUtilsMessenger () override;

    operator VkDebugUtilsMessengerEXT () const { return handle; }
};

} // namespace GVK

#endif