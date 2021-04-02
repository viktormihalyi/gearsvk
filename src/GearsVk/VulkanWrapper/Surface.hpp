#ifndef SURFACE_HPP
#define SURFACE_HPP

#include "GearsVkAPI.hpp"
#include "MovablePtr.hpp"
#include "VulkanObject.hpp"

#include <vulkan/vulkan.h>

namespace GVK {

class GVK_RENDERER_API Surface : public VulkanObject {
private:
    VkInstance                    instance;
    GVK::MovablePtr<VkSurfaceKHR> handle;

public:
    Surface (VkInstance instance, VkSurfaceKHR&& handle);

    enum PlatformSpecificSelector {
        PlatformSpecific
    };

    Surface (PlatformSpecificSelector, VkInstance instance, void* handle);

    virtual ~Surface () override;

    operator VkSurfaceKHR () const { return handle; }
};

} // namespace GVK

#endif