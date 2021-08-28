#include "VulkanEnvironment.hpp"

#include "Utils/BuildType.hpp"
#include "Utils/CommandLineFlag.hpp"
#include "Utils/StaticInit.hpp"
#include "Utils/Timer.hpp"

#include "Window.hpp"
#include "GLFWWindow.hpp"

#include "VulkanWrapper/Allocator.hpp"
#include "VulkanWrapper/DebugUtilsMessenger.hpp"
#include "VulkanWrapper/DeviceExtra.hpp"
#include "VulkanWrapper/Instance.hpp"
#include "VulkanWrapper/Surface.hpp"
#include "VulkanWrapper/Utils/VulkanUtils.hpp"
#include "VulkanWrapper/VulkanWrapper.hpp"


#include <iomanip>
#include <iostream>


static Utils::CommandLineOnOffFlag disableValidationLayersFlag (std::vector<std::string> { "--disableValidationLayers", "-v" }, "Disables Vulkan validation layers.");
static Utils::CommandLineOnOffFlag logVulkanVersionFlag ("--logVulkanVersion");


namespace RG {

Presentable::Presentable (VulkanEnvironment& env, std::unique_ptr<GVK::Surface>&& surface, std::unique_ptr<GVK::SwapchainSettingsProvider>&& settingsProvider)
    : surface (std::move (surface))
    , window (nullptr)
{
    // TODO this is kind of ugly
    // when creating a swapchain, we must query if presentation is supported for the surface
    // but we create a physicaldevice first and create the swapchains later
    // so we _hope_ it is supported

    GVK_ASSERT (env.CheckForPhsyicalDeviceSupport (*this));

    swapchain = std::make_unique<GVK::RealSwapchain> (*env.physicalDevice, *env.device, *this->surface, std::move (settingsProvider));
}


Presentable::Presentable (VulkanEnvironment& env, Window& window, std::unique_ptr<GVK::SwapchainSettingsProvider>&& settingsProvider)
    : Presentable (env, std::make_unique<GVK::Surface> (*env.instance, window.GetSurface (*env.instance)), std::move (settingsProvider))
{
}


Presentable::Presentable (VulkanEnvironment& env, std::unique_ptr<Window>&& window, std::unique_ptr<GVK::SwapchainSettingsProvider>&& settingsProvider)
    : Presentable (env, std::make_unique<GVK::Surface> (*env.instance, window->GetSurface (*env.instance)), std::move (settingsProvider))
{
    this->window = std::move (window);
}


GVK::Swapchain& Presentable::GetSwapchain ()
{
    return *swapchain;
}


const GVK::Surface& Presentable::GetSurface () const
{
    return *surface;
}


Window& Presentable::GetWindow ()
{
    GVK_ASSERT (window != nullptr);
    return *window;
}


std::optional<double> Presentable::GetRefreshRate () const
{
    if (window != nullptr) {
        return window->GetRefreshRate ();
    }

    return std::nullopt;
}



constexpr uint32_t LogColumnWidth = 36;


const std::unordered_map<VkObjectType, const char*> VkObjectTypeToStringMap {
    { VK_OBJECT_TYPE_UNKNOWN, "VK_OBJECT_TYPE_UNKNOWN" },
    { VK_OBJECT_TYPE_INSTANCE, "VK_OBJECT_TYPE_INSTANCE" },
    { VK_OBJECT_TYPE_PHYSICAL_DEVICE, "VK_OBJECT_TYPE_PHYSICAL_DEVICE" },
    { VK_OBJECT_TYPE_DEVICE, "VK_OBJECT_TYPE_DEVICE" },
    { VK_OBJECT_TYPE_QUEUE, "VK_OBJECT_TYPE_QUEUE" },
    { VK_OBJECT_TYPE_SEMAPHORE, "VK_OBJECT_TYPE_SEMAPHORE" },
    { VK_OBJECT_TYPE_COMMAND_BUFFER, "VK_OBJECT_TYPE_COMMAND_BUFFER" },
    { VK_OBJECT_TYPE_FENCE, "VK_OBJECT_TYPE_FENCE" },
    { VK_OBJECT_TYPE_DEVICE_MEMORY, "VK_OBJECT_TYPE_DEVICE_MEMORY" },
    { VK_OBJECT_TYPE_BUFFER, "VK_OBJECT_TYPE_BUFFER" },
    { VK_OBJECT_TYPE_IMAGE, "VK_OBJECT_TYPE_IMAGE" },
    { VK_OBJECT_TYPE_EVENT, "VK_OBJECT_TYPE_EVENT" },
    { VK_OBJECT_TYPE_QUERY_POOL, "VK_OBJECT_TYPE_QUERY_POOL" },
    { VK_OBJECT_TYPE_BUFFER_VIEW, "VK_OBJECT_TYPE_BUFFER_VIEW" },
    { VK_OBJECT_TYPE_IMAGE_VIEW, "VK_OBJECT_TYPE_IMAGE_VIEW" },
    { VK_OBJECT_TYPE_SHADER_MODULE, "VK_OBJECT_TYPE_SHADER_MODULE" },
    { VK_OBJECT_TYPE_PIPELINE_CACHE, "VK_OBJECT_TYPE_PIPELINE_CACHE" },
    { VK_OBJECT_TYPE_PIPELINE_LAYOUT, "VK_OBJECT_TYPE_PIPELINE_LAYOUT" },
    { VK_OBJECT_TYPE_RENDER_PASS, "VK_OBJECT_TYPE_RENDER_PASS" },
    { VK_OBJECT_TYPE_PIPELINE, "VK_OBJECT_TYPE_PIPELINE" },
    { VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT" },
    { VK_OBJECT_TYPE_SAMPLER, "VK_OBJECT_TYPE_SAMPLER" },
    { VK_OBJECT_TYPE_DESCRIPTOR_POOL, "VK_OBJECT_TYPE_DESCRIPTOR_POOL" },
    { VK_OBJECT_TYPE_DESCRIPTOR_SET, "VK_OBJECT_TYPE_DESCRIPTOR_SET" },
    { VK_OBJECT_TYPE_FRAMEBUFFER, "VK_OBJECT_TYPE_FRAMEBUFFER" },
    { VK_OBJECT_TYPE_COMMAND_POOL, "VK_OBJECT_TYPE_COMMAND_POOL" },
    { VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION, "VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION" },
    { VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE, "VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE" },
    { VK_OBJECT_TYPE_SURFACE_KHR, "VK_OBJECT_TYPE_SURFACE_KHR" },
    { VK_OBJECT_TYPE_SWAPCHAIN_KHR, "VK_OBJECT_TYPE_SWAPCHAIN_KHR" },
    { VK_OBJECT_TYPE_DISPLAY_KHR, "VK_OBJECT_TYPE_DISPLAY_KHR" },
    { VK_OBJECT_TYPE_DISPLAY_MODE_KHR, "VK_OBJECT_TYPE_DISPLAY_MODE_KHR" },
    { VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT, "VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT" },
    { VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT, "VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT" },
    { VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR, "VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR" },
    { VK_OBJECT_TYPE_VALIDATION_CACHE_EXT, "VK_OBJECT_TYPE_VALIDATION_CACHE_EXT" },
    { VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV, "VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV" },
    { VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL, "VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL" },
    { VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR, "VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR" },
    { VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV, "VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV" },
    { VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT, "VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT" },
    { VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR, "VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR" },
    { VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR, "VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR" },
};


static const char* VkObjectTypeToString (VkObjectType objectType)
{
    auto found = VkObjectTypeToStringMap.find (objectType);
    if (found != VkObjectTypeToStringMap.end ())
        return found->second;

    return "[UNKNOWN]";
}


// from https://www.lunarg.com/wp-content/uploads/2018/05/Vulkan-Debug-Utils_05_18_v1.pdf
void defaultDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                           VkDebugUtilsMessageTypeFlagsEXT             messageType,
                           const VkDebugUtilsMessengerCallbackDataEXT* callbackData)
{
    if (messageSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        return;
    }

    char  prefix[64];
    char* message = (char*)malloc (strlen (callbackData->pMessage) + 500);
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        strcpy (prefix, "VERBOSE : ");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        strcpy (prefix, "INFO : ");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        strcpy (prefix, "WARNING : ");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        strcpy (prefix, "ERROR : ");
    }
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        strcat (prefix, "GENERAL");
    } else {
#if 0
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_SPECIFICATION_BIT_EXT) {
            strcat (prefix, "SPEC");
            // validation_error = 1;
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_SPECIFICATION_BIT_EXT) {
                strcat (prefix, "|");
            }
        }
#endif
    }

    sprintf (message,
             "\n%sMessage ID Number %d, Message ID String : %s\n\t%s",
             prefix,
             callbackData->messageIdNumber,
             callbackData->pMessageIdName,
             callbackData->pMessage);
    if (callbackData->objectCount > 0) {
        char tmp_message[500];
        sprintf (tmp_message, "\n\n\tObjects - %d\n", callbackData->objectCount);
        strcat (message, tmp_message);
        for (uint32_t object = 0; object < callbackData->objectCount; ++object) {
            sprintf (tmp_message,
                     "\t\tObject[%d] - Type %s, Value %#p, Name \"%s\"\n",
                     object,
                     VkObjectTypeToString (callbackData->pObjects[object].objectType),
                     (void*)(callbackData->pObjects[object].objectHandle),
                     callbackData->pObjects[object].pObjectName == nullptr ? "[UNNAMED]" : callbackData->pObjects[object].pObjectName);
            strcat (message, tmp_message);
        }
    }
    if (callbackData->cmdBufLabelCount > 0) {
        char tmp_message[500];
        sprintf (tmp_message,
                 "\n\n\tCommand Buffer Labels - %d\n",
                 callbackData->cmdBufLabelCount);
        strcat (message, tmp_message);
        for (uint32_t label = 0; label < callbackData->cmdBufLabelCount; ++label) {
            sprintf (tmp_message,
                     "\t\tLabel[%d] - %s { %f, %f, %f, %f}\n",
                     label,
                     callbackData->pCmdBufLabels[label].pLabelName,
                     callbackData->pCmdBufLabels[label].color[0],
                     callbackData->pCmdBufLabels[label].color[1],
                     callbackData->pCmdBufLabels[label].color[2],
                     callbackData->pCmdBufLabels[label].color[3]);
            strcat (message, tmp_message);
        }
    }
    printf ("%s\n", message);
    fflush (stdout);
    free (message);
}


void VulkanEnvironment::Wait () const
{
    graphicsQueue->Wait ();
    device->Wait ();
}


DebugOnlyStaticInit apiVersionLogger ([] () {
    uint32_t apiVersion;
    vkEnumerateInstanceVersion (&apiVersion);
    std::cout << std::left << std::setw (LogColumnWidth) << "vulkan api version:" << GVK::GetVersionString (apiVersion) << std::endl;
});


VulkanEnvironment::VulkanEnvironment (std::optional<GVK::DebugUtilsMessenger::Callback> callback, const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& deviceExtensions)
{
    GVK::InstanceSettings is = (IsDebugBuild) ? GVK::instanceDebugMode : GVK::instanceReleaseMode;
    is.extensions.insert (is.extensions.end (), instanceExtensions.begin (), instanceExtensions.end ());

    instance = std::make_unique<GVK::Instance> (is);

    if (IsDebugBuild && callback.has_value () && !disableValidationLayersFlag.IsFlagOn ()) {
        messenger = std::make_unique<GVK::DebugUtilsMessenger> (*instance, *callback, GVK::DebugUtilsMessenger::noPerformance);
    }

    const VkSurfaceKHR physicalDeviceSurfaceHandle = VK_NULL_HANDLE;

    physicalDevice = std::make_unique<GVK::PhysicalDevice> (*instance, physicalDeviceSurfaceHandle, std::set<std::string> {});

    if (logVulkanVersionFlag.IsFlagOn ()) {
        VkPhysicalDeviceProperties properties = physicalDevice->GetProperties ();

        auto DeviceTypeToString = [] (VkPhysicalDeviceType type) -> std::string {
            switch (type) {
                case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "VK_PHYSICAL_DEVICE_TYPE_OTHER";
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
                case VK_PHYSICAL_DEVICE_TYPE_CPU: return "VK_PHYSICAL_DEVICE_TYPE_CPU";
                default:
                    GVK_ASSERT (false);
                    return "<unknown>";
            }
        };

        std::cout << std::left << std::setw (LogColumnWidth) << "physical device api version:" << GVK::GetVersionString (properties.apiVersion) << std::endl;
        std::cout << std::left << std::setw (LogColumnWidth) << "physical device driver version:" << GVK::GetVersionString (properties.driverVersion) << " (" << properties.driverVersion << ")" << std::endl;
        std::cout << std::left << std::setw (LogColumnWidth) << "device name:" << properties.deviceName << std::endl;
        std::cout << std::left << std::setw (LogColumnWidth) << "device type:" << DeviceTypeToString (properties.deviceType) << std::endl;
        std::cout << std::left << std::setw (LogColumnWidth) << "device id:" << properties.deviceID << std::endl;
        std::cout << std::left << std::setw (LogColumnWidth) << "vendor id:" << properties.vendorID << std::endl;
        std::cout << std::endl;

        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties (*physicalDevice, VK_FORMAT_R32_SFLOAT, &props);
        vkGetPhysicalDeviceFormatProperties (*physicalDevice, VK_FORMAT_R32G32B32_SFLOAT, &props);
        vkGetPhysicalDeviceFormatProperties (*physicalDevice, VK_FORMAT_R32G32B32_SFLOAT, &props);
    }

    device = std::make_unique<GVK::DeviceObject> (*physicalDevice, std::vector<uint32_t> { *physicalDevice->GetQueueFamilies ().graphics }, deviceExtensions);

    allocator = std::make_unique<GVK::Allocator> (*instance, *physicalDevice, *device);

    graphicsQueue = std::make_unique<GVK::Queue> (*device, *physicalDevice->GetQueueFamilies ().graphics);

    commandPool = std::make_unique<GVK::CommandPool> (*device, *physicalDevice->GetQueueFamilies ().graphics);

    deviceExtra = std::make_unique<GVK::DeviceExtra> (*instance, *device, *commandPool, *allocator, *graphicsQueue);

    commandPool->SetName (*deviceExtra, "VulkanEnvironment CommandPool");
    static_cast<GVK::DeviceObject*> (device.get ())->SetName (*deviceExtra, "VulkanEnvironment DeviceObject");

    if (logVulkanVersionFlag.IsFlagOn ()) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties (*physicalDevice, &deviceProperties);

        std::cout << "physical device api version: " << GVK::GetVersionString (deviceProperties.apiVersion) << std::endl;
        std::cout << "physical device driver version: " << GVK::GetVersionString (deviceProperties.driverVersion) << std::endl;
    }
}


VulkanEnvironment::~VulkanEnvironment ()
{
    Wait ();
}


bool VulkanEnvironment::CheckForPhsyicalDeviceSupport (const Presentable& presentable)
{
    const GVK::Surface& surface = presentable.GetSurface ();
    return physicalDevice->CheckSurfaceSupported (surface);
}

} // namespace RG