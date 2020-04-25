#include "VulkanEnvironment.hpp"

#include "StaticInit.hpp"
#include "Timer.hpp"


void VulkanEnvironment::Wait () const
{
    graphicsQueue->Wait ();
    device->Wait ();
}


StaticInit apiVersionLogger ([] () {
    uint32_t apiVersion;
    vkEnumerateInstanceVersion (&apiVersion);
    std::cout << "api version: " << GetVersionString (apiVersion) << std::endl;
});


VulkanEnvironment::VulkanEnvironment (VulkanEnvironment::Mode mode, std::optional<Window::Ref> window, std::optional<DebugUtilsMessenger::Callback> callback)
{
    Utils::TimerLogger tl ("creating test environment");
    Utils::TimerScope  ts (tl);

    ASSERT (mode == Mode::Debug || mode == Mode::Release);

    InstanceSettings is = (mode == Mode::Debug) ? instanceDebugMode : instanceReleaseMode;

    if (window) {
        auto windowExtenions = window->get ().GetExtensions ();
        is.extensions.insert (is.extensions.end (), windowExtenions.begin (), windowExtenions.end ());
    }

    instance = Instance::Create (is);

    if (window) {
        surface = Surface::Create (*instance, window->get ().GetSurface (*instance));
    }

    if (mode == Mode::Debug && callback.has_value ()) {
        messenger = DebugUtilsMessenger::Create (*instance, *callback, DebugUtilsMessenger::noPerformance);
    }

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


VulkanEnvironment::U VulkanEnvironment::CreateForBuildType (std::optional<Window::Ref> window, std::optional<DebugUtilsMessenger::Callback> callback)
{
#ifdef NDEBUG
    return ReleaseVulkanEnvironment::Create (window);
#else
    return DebugVulkanEnvironment::Create (window, callback);
#endif
}
