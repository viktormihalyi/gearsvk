#ifndef WINDOWBASE_HPP
#define WINDOWBASE_HPP

#include "Noncopyable.hpp"
#include "Ptr.hpp"

#include <functional>
#include <vector>

#include <vulkan/vulkan.h>


class WindowBase : public Noncopyable {
public:
    USING_PTR (WindowBase);

    using DrawCallback = std::function<void ()>;

    virtual ~WindowBase ()
    {
    }

    virtual void  DoEventLoop (const DrawCallback&) = 0;
    virtual void* GetHandle () const                = 0;

    virtual std::vector<const char*> GetExtensions () const                    = 0;
    virtual VkSurfaceKHR             CreateSurface (VkInstance instance) const = 0;

    virtual uint32_t GetWidth () const  = 0;
    virtual uint32_t GetHeight () const = 0;
};

#endif