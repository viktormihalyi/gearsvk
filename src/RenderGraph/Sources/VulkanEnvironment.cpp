#include "VulkanEnvironment.hpp"

#include "Utils/BuildType.hpp"
#include "Utils/CommandLineFlag.hpp"
#include "Utils/StaticInit.hpp"
#include "Utils/TerminalColors.hpp"
#include "Utils/Timer.hpp"

#include "Window.hpp"

#include "VulkanWrapper/Allocator.hpp"
#include "VulkanWrapper/DebugUtilsMessenger.hpp"
#include "VulkanWrapper/DeviceExtra.hpp"
#include "VulkanWrapper/Instance.hpp"
#include "VulkanWrapper/Surface.hpp"
#include "VulkanWrapper/Utils/VulkanUtils.hpp"
#include "VulkanWrapper/VulkanWrapper.hpp"


#include <iomanip>


static Utils::CommandLineOnOffFlag disableValidationLayersFlag (std::vector<std::string> { "--disableValidationLayers", "-v" }, "Disables Vulkan validation layers.");
static Utils::CommandLineOnOffFlag logVulkanVersionFlag ("--logVulkanVersion");


namespace GVK {

Presentable::Presentable (VulkanEnvironment& env, std::unique_ptr<Surface>&& surface, SwapchainSettingsProvider& settingsProvider)
    : surface (std::move (surface))
    , window (nullptr)
{
    // TODO this is kind of a hack
    // when creating a swapchain, we must query if presentation is supported for the surface
    // but we create a physicaldevice first, then connect the swapchains later, so we completely recreate the physical device
    // and _hope_ we get the same

    env.RecreateForPresentable (*this);

    swapchain = std::make_unique<RealSwapchain> (*env.physicalDevice, *env.device, *this->surface, settingsProvider);
}


Presentable::Presentable (VulkanEnvironment& env, Window& window, SwapchainSettingsProvider& settingsProvider)
    : Presentable (env, std::make_unique<Surface> (*env.instance, window.GetSurface (*env.instance)), settingsProvider)
{
}


Presentable::Presentable (VulkanEnvironment& env, std::unique_ptr<Window>&& window, SwapchainSettingsProvider& settingsProvider)
    : Presentable (env, std::make_unique<Surface> (*env.instance, window->GetSurface (*env.instance)), settingsProvider)
{
    this->window = std::move (window);
}


Swapchain& Presentable::GetSwapchain ()
{
    return *swapchain;
}


const Surface& Presentable::GetSurface () const
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


void testDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
                        VkDebugUtilsMessageTypeFlagsEXT             messageType,
                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData)
{
    using namespace TerminalColors;

    if (severity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        std::cout << RED << "validation layer: "
                  << YELLOW << callbackData->pMessageIdName << ": "
                  << RESET << callbackData->pMessage
                  << std::endl
                  << std::endl;
    }
}


void VulkanEnvironment::Wait () const
{
    graphicsQueue->Wait ();
    device->Wait ();
}


DebugOnlyStaticInit apiVersionLogger ([] () {
    uint32_t apiVersion;
    vkEnumerateInstanceVersion (&apiVersion);
    std::cout << std::left << std::setw (LogColumnWidth) << "vulkan api version:" << GetVersionString (apiVersion) << std::endl;
});


VulkanEnvironment::VulkanEnvironment (std::optional<DebugUtilsMessenger::Callback> callback)
{
    InstanceSettings is = (IsDebugBuild) ? instanceDebugMode : instanceReleaseMode;

    const std::vector<const char*> windowExtenions = Window::GetExtensions ();
    is.extensions.insert (is.extensions.end (), windowExtenions.begin (), windowExtenions.end ());

    instance = std::make_unique<Instance> (is);

    if (IsDebugBuild && callback.has_value () && !disableValidationLayersFlag.IsFlagOn ()) {
        messenger = std::make_unique<DebugUtilsMessenger> (*instance, *callback, DebugUtilsMessenger::noPerformance);
        debugReportCallback = std::make_unique<DebugReportCallback> (*instance);
    }

    VkSurfaceKHR physicalDeviceSurfaceHandle = VK_NULL_HANDLE;

    physicalDevice = std::make_unique<PhysicalDevice> (*instance, physicalDeviceSurfaceHandle, std::set<std::string> {});

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

        std::cout << std::left << std::setw (LogColumnWidth) << "physical device api version:" << GetVersionString (properties.apiVersion) << std::endl;
        std::cout << std::left << std::setw (LogColumnWidth) << "physical device driver version:" << GetVersionString (properties.driverVersion) << " (" << properties.driverVersion << ")" << std::endl;
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

    std::vector<const char*> deviceExtensions;

    deviceExtensions.push_back (VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    device = std::make_unique<DeviceObject> (*physicalDevice, std::vector<uint32_t> { *physicalDevice->GetQueueFamilies ().graphics }, deviceExtensions);

    allocator = std::make_unique<Allocator> (*instance, *physicalDevice, *device);

    graphicsQueue = std::make_unique<Queue> (*device, *physicalDevice->GetQueueFamilies ().graphics);

    commandPool = std::make_unique<CommandPool> (*device, *physicalDevice->GetQueueFamilies ().graphics);

    deviceExtra = std::make_unique<DeviceExtra> (*device, *commandPool, *allocator, *graphicsQueue);

    if (logVulkanVersionFlag.IsFlagOn ()) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties (*physicalDevice, &deviceProperties);

        std::cout << "physical device api version: " << GetVersionString (deviceProperties.apiVersion) << std::endl;
        std::cout << "physical device driver version: " << GetVersionString (deviceProperties.driverVersion) << std::endl;
    }
}


VulkanEnvironment::~VulkanEnvironment ()
{
    Wait ();
}


void VulkanEnvironment::RecreateForPresentable (const Presentable& presentable)
{
    const Surface& surface = presentable.GetSurface ();
    physicalDevice->RecreateForSurface (surface);
}

} // namespace GVK