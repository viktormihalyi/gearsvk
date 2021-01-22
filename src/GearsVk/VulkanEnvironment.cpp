#include "VulkanEnvironment.hpp"

#include "BuildType.hpp"
#include "StaticInit.hpp"
#include "TerminalColors.hpp"
#include "Timer.hpp"
#include "Window.hpp"

#include "Allocator.hpp"
#include "DebugUtilsMessenger.hpp"
#include "DeviceExtra.hpp"
#include "Instance.hpp"
#include "Surface.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"


#include <iomanip>


Presentable::Presentable (VulkanEnvironment& env, SurfaceU&& surface, SwapchainSettingsProvider& settingsProvider)
    : surface (std::move (surface))
    , window (nullptr)
{
    // TODO this is kind of a hack
    // when creating a swapchain, we must query if presentation is supported for the surface
    // but we create a physicaldevice first, then connect the swapchains later, so we completely recreate the physical device
    // and _hope_ we get the same

    env.RecreateForPresentable (*this);

    swapchain = RealSwapchain::Create (*env.physicalDevice, *env.device, *this->surface, settingsProvider);
}


Presentable::Presentable (VulkanEnvironment& env, Window& window, SwapchainSettingsProvider& settingsProvider)
    : Presentable (env, Surface::Create (*env.instance, window.GetSurface (*env.instance)), settingsProvider)
{
}


Presentable::Presentable (VulkanEnvironment& env, WindowU&& window, SwapchainSettingsProvider& settingsProvider)
    : Presentable (env, Surface::Create (*env.instance, window->GetSurface (*env.instance)), settingsProvider)
{
    window = std::move (window);
}


Swapchain& Presentable::GetSwapchain ()
{
    return *swapchain;
}


const Surface& Presentable::GetSurface () const
{
    return *surface;
}


constexpr uint32_t LogColumnWidth = 36;


void testDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
                        VkDebugUtilsMessageTypeFlagsEXT             messageType,
                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData)
{
    using namespace TerminalColors;

    if (severity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        std::cout << callbackData->pMessage << std::endl
                  << std::endl;
        //std::cout << RED << "validation layer: "
        //          << YELLOW << callbackData->pMessageIdName << ": "
        //          << RESET << callbackData->pMessage
        //          << std::endl
        //          << std::endl;
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

    instance = Instance::Create (is);

    if (IsDebugBuild && callback.has_value ()) {
        messenger = DebugUtilsMessenger::Create (*instance, *callback, DebugUtilsMessenger::noPerformance);
    }

    VkSurfaceKHR physicalDeviceSurfaceHandle = VK_NULL_HANDLE;

    physicalDevice = PhysicalDevice::Create (*instance, physicalDeviceSurfaceHandle, std::set<std::string> {});

#ifdef LOG_VULKAN_INFO
    {
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
#endif

    std::vector<const char*> deviceExtensions;

    deviceExtensions.push_back (VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    device = DeviceObject::Create (*physicalDevice, std::vector<uint32_t> { *physicalDevice->GetQueueFamilies ().graphics }, deviceExtensions);

    alloactor = Allocator::Create (*instance, *physicalDevice, *device);

    graphicsQueue = Queue::Create (*device, *physicalDevice->GetQueueFamilies ().graphics);

    commandPool = CommandPool::Create (*device, *physicalDevice->GetQueueFamilies ().graphics);

    deviceExtra = DeviceExtra::Create (*device, *commandPool, *alloactor, *graphicsQueue);

#ifdef LOG_VULKAN_INFO
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties (*physicalDevice, &deviceProperties);

    std::cout << "physical device api version: " << GetVersionString (deviceProperties.apiVersion) << std::endl;
    std::cout << "physical device driver version: " << GetVersionString (deviceProperties.driverVersion) << std::endl;
#endif
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