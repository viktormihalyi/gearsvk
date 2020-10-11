#ifndef SURFACE_HPP
#define SURFACE_HPP

#include "GearsVkAPI.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"

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

    enum PlatformSelector {
        ForWin32
    };


#ifdef _WIN32
    Surface (PlatformSelector, VkInstance instance, void* handle);
#endif

    virtual ~Surface ()
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