#ifndef SURFACE_HPP
#define SURFACE_HPP

#include "GearsVkAPI.hpp"
#include "VulkanObject.hpp"
#include "Ptr.hpp"

#include <vulkan/vulkan.h>


USING_PTR (Surface);
class GVK_RENDERER_API Surface : public VulkanObject {
    USING_CREATE (Surface);

private:
    VkInstance   instance;
    VkSurfaceKHR handle;

public:
    Surface (VkInstance instance, VkSurfaceKHR&& handle);

    enum PlatformSpecificSelector {
        PlatformSpecific
    };

    Surface (PlatformSpecificSelector, VkInstance instance, void* handle);

    virtual ~Surface () override;

    operator VkSurfaceKHR () const { return handle; }
};

#endif