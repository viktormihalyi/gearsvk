#ifndef PHYSICALDEVICE_HPP
#define PHYSICALDEVICE_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"
#include "VulkanUtils.hpp"

#include <vulkan/vulkan.h>


class QueueFamilySelector {
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


class DefaultQueueFamilySelector final : public QueueFamilySelector {
public:
    DefaultQueueFamilySelector ();
};


extern DefaultQueueFamilySelector defaultQueueFamilySelector;


class PhysicalDevice final : public Noncopyable {
public:
    struct QueueFamilies {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> presentation;
        std::optional<uint32_t> transfer;
        std::optional<uint32_t> compute;
    };

private:
    const VkInstance            instance;
    const std::set<std::string> requestedDeviceExtensionSet;
    const QueueFamilySelector   selector;

    VkPhysicalDevice handle;
    QueueFamilies    queueFamilies;

public:
    USING_PTR (PhysicalDevice);

    PhysicalDevice (VkInstance instance, VkSurfaceKHR surface, const std::set<std::string>& requestedDeviceExtensionSet, const QueueFamilySelector& Selector = defaultQueueFamilySelector);

    ~PhysicalDevice ();

    void RecreateForSurface (VkSurfaceKHR surface);

    operator VkPhysicalDevice () const { return handle; }

    QueueFamilies GetQueueFamilies () const { return queueFamilies; }
};

#endif