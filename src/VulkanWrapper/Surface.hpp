#ifndef SURFACE_HPP
#define SURFACE_HPP

#include "Noncopyable.hpp"

#include <vulkan/vulkan.h>

class Surface : public Noncopyable {
private:
    VkInstance   instance;
    VkSurfaceKHR handle;

public:
    Surface (VkInstance instance, VkSurfaceKHR handle)
        : instance (instance)
        , handle (handle)
    {
    }

    ~Surface ()
    {
        vkDestroySurfaceKHR (instance, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkSurfaceKHR () const
    {
        return handle;
    }
};

#endif