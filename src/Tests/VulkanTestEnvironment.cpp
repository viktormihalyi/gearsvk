#include "VulkanTestEnvironment.hpp"

#include "Timer.hpp"

void TestEnvironment::Wait () const
{
    graphicsQueue->Wait ();
    device->Wait ();
}


TestEnvironment::TestEnvironment (std::vector<const char*> instanceExtensions, std::optional<Window::Ref> window, DebugUtilsMessenger::Callback callback)
{
    Utils::TimerLogger tl ("creating test environment");
    Utils::TimerScope  ts (tl);

    if (window) {
        auto windowExtenions = window->get ().GetExtensions ();
        instanceExtensions.insert (instanceExtensions.end (), windowExtenions.begin (), windowExtenions.end ());
    }

    instance = Instance::Create (instanceExtensions, std::vector<const char*> {"VK_LAYER_KHRONOS_validation"});

    if (window) {
        surface = Surface::Create (*instance, window->get ().GetSurface (*instance));
    }

    messenger = DebugUtilsMessenger::Create (*instance, callback, DebugUtilsMessenger::noPerformance);

    VkSurfaceKHR physicalDeviceSurfaceHandle = ((surface != nullptr) ? surface->operator VkSurfaceKHR () : VK_NULL_HANDLE);

    physicalDevice = PhysicalDevice::Create (*instance, physicalDeviceSurfaceHandle, std::set<std::string> {});

    std::vector<const char*> deviceExtensions;

    if (window) {
        deviceExtensions.push_back (VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    device = Device::Create (*physicalDevice, std::vector<uint32_t> {*physicalDevice->GetQueueFamilies ().graphics}, deviceExtensions);

    graphicsQueue = Queue::Create (*device, *physicalDevice->GetQueueFamilies ().graphics);

    commandPool = CommandPool::Create (*device, *physicalDevice->GetQueueFamilies ().graphics);

    if (window) {
        swapchain = RealSwapchain::Create (*physicalDevice, *device, *surface);
    } else {
        swapchain = FakeSwapchain::Create (*device, *graphicsQueue, *commandPool, 512, 512);
    }

#ifdef TESTENV_LOG_VERSION
    uint32_t apiVersion;
    vkEnumerateInstanceVersion (&apiVersion);
    std::cout << "instance api version: " << GetVersionString (apiVersion) << std::endl;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties (*physicalDevice, &deviceProperties);

    std::cout << "physical device api version: " << GetVersionString (deviceProperties.apiVersion) << std::endl;
    std::cout << "physical device driver version: " << GetVersionString (deviceProperties.driverVersion) << std::endl;
#endif
}
