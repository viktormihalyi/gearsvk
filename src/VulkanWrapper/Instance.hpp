#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"
#include "VulkanUtils.hpp"

#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

struct InstanceSettings {
    std::vector<const char*> extensions;
    std::vector<const char*> layers;
};

static const InstanceSettings instanceDebugMode {{VK_EXT_DEBUG_UTILS_EXTENSION_NAME}, {"VK_LAYER_KHRONOS_validation"}};
static const InstanceSettings instanceReleaseMode {{}, {}};

class Instance : public Noncopyable {
private:
    VkInstance handle;

    static VkInstance CreateInstance (const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& instanceLayers)
    {
        auto extensionNameAccessor = [] (const VkExtensionProperties& props) { return props.extensionName; };
        auto layerNameAccessor     = [] (const VkLayerProperties& props) { return props.layerName; };

        const std::set<std::string> requiredExtensionSet = Utils::ToSet<const char*, std::string> (instanceExtensions);

        // supported extensions
        std::set<std::string> supportedExtensionSet;
        {
            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> supportedExtensions (extensionCount);
            vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, supportedExtensions.data ());

            supportedExtensionSet = Utils::ToSet<VkExtensionProperties, std::string> (supportedExtensions, extensionNameAccessor);
        }

        // check if the required extensions are supported
        {
            const std::set<std::string> unsupportedExtensionSet = Utils::SetDiff (requiredExtensionSet, supportedExtensionSet);
            if (ERROR (!unsupportedExtensionSet.empty ())) {
                throw std::runtime_error ("not all instance extensions are supported");
            }
        }


        std::set<std::string> requiredValidationLayerSet = Utils::ToSet<const char*, std::string> (instanceLayers);

        // supported validation layers
        std::set<std::string> supportedValidationLayerSet;
        {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties (&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers (layerCount);
            vkEnumerateInstanceLayerProperties (&layerCount, availableLayers.data ());

            supportedValidationLayerSet = Utils::ToSet<VkLayerProperties, std::string> (availableLayers, layerNameAccessor);
        }

        // check if the required validation layers are supported
        {
            const std::set<std::string> unsupportedValidationLayerSet = Utils::SetDiff (requiredValidationLayerSet, supportedValidationLayerSet);
            if (ERROR (!unsupportedValidationLayerSet.empty ())) {
                throw std::runtime_error ("not all validation layers are supported");
            }
        }


        VkApplicationInfo appInfo  = {};
        appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName   = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION (1, 1, 0);
        appInfo.pEngineName        = "No Engine";
        appInfo.engineVersion      = VK_MAKE_VERSION (1, 1, 0);
        appInfo.apiVersion         = VK_API_VERSION_1_1;

        VkInstanceCreateInfo createInfo    = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo        = &appInfo;
        createInfo.enabledExtensionCount   = static_cast<uint32_t> (instanceExtensions.size ());
        createInfo.ppEnabledExtensionNames = instanceExtensions.data ();
        createInfo.enabledLayerCount       = static_cast<uint32_t> (instanceLayers.size ());
        createInfo.ppEnabledLayerNames     = instanceLayers.data ();

        VkInstance instance = VK_NULL_HANDLE;
        VkResult   result   = vkCreateInstance (&createInfo, nullptr, &instance);
        if (ERROR (result != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create vulkan instance");
        }

        return instance;
    }

public:
    USING_PTR (Instance);

    Instance (const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& instanceLayers)
        : handle (CreateInstance (instanceExtensions, instanceLayers))
    {
    }

    Instance (const InstanceSettings& settings)
        : handle (CreateInstance (settings.extensions, settings.layers))
    {
    }

    ~Instance ()
    {
        vkDestroyInstance (handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkInstance () const
    {
        return handle;
    }
};

#endif