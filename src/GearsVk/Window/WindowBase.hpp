#ifndef WINDOWBASE_HPP
#define WINDOWBASE_HPP

#include "Event.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"

#include <functional>
#include <vector>

#include <vulkan/vulkan.h>


class WindowBase : public Noncopyable {
public:
    struct {
        // window events
        Event<>                   shown;
        Event<>                   hidden;
        Event<>                   closed;
        Event<>                   focused;
        Event<>                   focusLost;
        Event<uint32_t, uint32_t> resized;
        Event<uint32_t, uint32_t> moved;

        // user input
        Event<uint32_t>           keyPressed;
        Event<uint32_t>           keyReleased;
        Event<uint32_t, uint32_t> mouseMove;
        Event<uint32_t, uint32_t> leftMouseButtonPressed;
        Event<uint32_t, uint32_t> leftMouseButtonReleased;
        Event<uint32_t, uint32_t> rightMouseButtonPressed;
        Event<uint32_t, uint32_t> rightMouseButtonReleased;
        Event<int32_t>            scroll;
    } events;

public:
    USING_PTR (WindowBase);

    using DrawCallback = std::function<void (bool& stopFlag)>;

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