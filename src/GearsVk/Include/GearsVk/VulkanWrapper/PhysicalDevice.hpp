#ifndef PHYSICALDEVICE_HPP
#define PHYSICALDEVICE_HPP

#include "GearsVkAPI.hpp"

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "Utils/Utils.hpp"
#include "VulkanUtils.hpp"

#include <vulkan/vulkan.h>

#include <functional>
#include <optional>
#include <vector>

namespace GVK {

class GVK_RENDERER_API QueueFamilySelector {
public:
    using Selector = std::function<std::optional<uint32_t> (VkPhysicalDevice, VkSurfaceKHR, const std::vector<VkQueueFamilyProperties>&)>;

public:
    Selector graphicsSelector;
    Selector presentationSelector;
    Selector computeSelector;
    Selector transferSelector;

    QueueFamilySelector (const Selector& graphicsSelector,
                         const Selector& presentationSelector,
                         const Selector& computeSelector,
                         const Selector& transferSelector)
        : graphicsSelector (graphicsSelector)
        , presentationSelector (presentationSelector)
        , computeSelector (computeSelector)
        , transferSelector (transferSelector)
    {
    }

    virtual ~QueueFamilySelector () {}
};


class GVK_RENDERER_API DefaultQueueFamilySelector final : public QueueFamilySelector {
public:
    DefaultQueueFamilySelector ();
};


GVK_RENDERER_API
extern DefaultQueueFamilySelector defaultQueueFamilySelector;


class GVK_RENDERER_API PhysicalDevice final : public Noncopyable {
public:
    struct QueueFamilies {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> presentation;
        std::optional<uint32_t> transfer;
        std::optional<uint32_t> compute;
    };

private:
    VkInstance            instance;
    std::set<std::string> requestedDeviceExtensionSet;
    QueueFamilySelector   selector;

    GVK::MovablePtr<VkPhysicalDevice> handle;
    QueueFamilies                     queueFamilies;

    std::unique_ptr<VkPhysicalDeviceProperties> properties;
    std::unique_ptr<VkPhysicalDeviceFeatures> features;

public:
    PhysicalDevice (VkInstance instance, VkSurfaceKHR surface, const std::set<std::string>& requestedDeviceExtensionSet, const QueueFamilySelector& Selector = defaultQueueFamilySelector);

    ~PhysicalDevice ();

    void RecreateForSurface (VkSurfaceKHR surface);

    operator VkPhysicalDevice () const { return handle; }

    QueueFamilies GetQueueFamilies () const { return queueFamilies; }

    VkPhysicalDeviceProperties GetProperties () const
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties (handle, &properties);
        return properties;
    }

    VkPhysicalDeviceFeatures GetFeatures () const
    {
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures (handle, &features);
        return features;
    }
};

} // namespace GVK

#endif