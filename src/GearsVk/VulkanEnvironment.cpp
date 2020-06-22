#include "VulkanEnvironment.hpp"

#include "BuildType.hpp"
#include "StaticInit.hpp"
#include "Timer.hpp"

#include <iomanip>


constexpr uint32_t LogColumnWidth = 36;


void testDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT,
                        VkDebugUtilsMessageTypeFlagsEXT,
                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData)
{
    using namespace TerminalColors;
    std::cout << callbackData->pMessage << std::endl
              << std::endl;
    //std::cout << RED << "validation layer: "
    //          << YELLOW << callbackData->pMessageIdName << ": "
    //          << RESET << callbackData->pMessage
    //          << std::endl
    //          << std::endl;
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


VulkanEnvironment::VulkanEnvironment (std::optional<WindowRef> window, std::optional<DebugUtilsMessenger::Callback> callback)
{
    Utils::DebugTimerLogger tl ("creating test environment");
    Utils::TimerScope       ts (tl);

    InstanceSettings is = (IsDebugBuild) ? instanceDebugMode : instanceReleaseMode;

    if (window) {
        auto windowExtenions = window->get ().GetExtensions ();
        is.extensions.insert (is.extensions.end (), windowExtenions.begin (), windowExtenions.end ());
    }

    instance = Instance::Create (is);

    if (window) {
        surface = Surface::Create (*instance, window->get ().GetSurface (*instance));
    }

    if (IsDebugBuild && callback.has_value ()) {
        messenger = DebugUtilsMessenger::Create (*instance, *callback, DebugUtilsMessenger::noPerformance);
    }

    VkSurfaceKHR physicalDeviceSurfaceHandle = ((surface != nullptr) ? surface->operator VkSurfaceKHR () : VK_NULL_HANDLE);

    physicalDevice = PhysicalDevice::Create (*instance, physicalDeviceSurfaceHandle, std::set<std::string> {});

    if constexpr (IsDebugBuild) {
        VkPhysicalDeviceProperties properties = physicalDevice->GetProperties ();

        auto DeviceTypeToString = [] (VkPhysicalDeviceType type) -> std::string {
            switch (type) {
                case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "VK_PHYSICAL_DEVICE_TYPE_OTHER";
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
                case VK_PHYSICAL_DEVICE_TYPE_CPU: return "VK_PHYSICAL_DEVICE_TYPE_CPU";
                default:
                    ASSERT (false);
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
    }

    std::vector<const char*> deviceExtensions;

    if (window) {
        deviceExtensions.push_back (VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    device = Device::Create (*physicalDevice, std::vector<uint32_t> {*physicalDevice->GetQueueFamilies ().graphics}, deviceExtensions);

    graphicsQueue = Queue::Create (*device, *physicalDevice->GetQueueFamilies ().graphics);

    commandPool = CommandPool::Create (*device, *physicalDevice->GetQueueFamilies ().graphics);

    deviceExtra = DeviceExtra::Create (*device, *commandPool, *graphicsQueue);

    if (window) {
        swapchain = RealSwapchain::Create (*physicalDevice, *device, *surface);
    } else {
        swapchain = FakeSwapchain::Create (*device, *graphicsQueue, *commandPool, 512, 512);
    }

#ifdef TESTENV_LOG_VERSION
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
