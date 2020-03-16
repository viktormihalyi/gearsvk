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

    DebugUtilsMessenger (VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT callback, const Settings& settings = defaultSettings);

    ~DebugUtilsMessenger ();

    operator VkDebugUtilsMessengerEXT () const { return handle; }
};

#endif