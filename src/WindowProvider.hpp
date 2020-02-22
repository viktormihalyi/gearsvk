#ifndef WINDOWPROVIDER_HPP
#define WINDOWPROVIDER_HPP

#include "Ptr.hpp"

#include <functional>
#include <vector>

#include <vulkan/vulkan.h>


DEFINE_PTR (WindowProvider);

class WindowProvider {
public:
    using DrawCallback = std::function<void ()>;

    virtual ~WindowProvider ()
    {
    }

    virtual void  DoEventLoop (const DrawCallback&) = 0;
    virtual void* GetHandle () const                = 0;

    virtual std::vector<const char*> GetExtensions () const                    = 0;
    virtual VkSurfaceKHR             CreateSurface (VkInstance instance) const = 0;
};

#endif