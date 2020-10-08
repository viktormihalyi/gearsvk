#ifndef SURFACE_HPP
#define SURFACE_HPP

#include "Noncopyable.hpp"

#include <vulkan/vulkan.h>

USING_PTR (Surface);
class GEARSVK_API Surface : public Noncopyable {
private:
    VkInstance   instance;
    VkSurfaceKHR handle;

public:
    USING_CREATE (Surface);

    Surface (VkInstance instance, VkSurfaceKHR&& handle)
        : instance (instance)
        , handle (handle)
    {
    }

    ~Surface ()
    {
        if (handle != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR (instance, handle, nullptr);
            handle = VK_NULL_HANDLE;
        }
    }

    operator VkSurfaceKHR () const
    {
        return handle;
    }
};

#endif