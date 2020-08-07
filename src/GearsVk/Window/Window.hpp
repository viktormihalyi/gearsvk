#ifndef WINDOWB_HPP
#define WINDOWB_HPP

#include "Event.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"

#include <functional>
#include <vector>

#include <vulkan/vulkan.h>

USING_PTR (Window);

class GEARSVK_API Window : public Noncopyable {
public:
    struct Events {
        // window events
        Event<>                   shown;
        Event<>                   hidden;
        Event<>                   closed;
        Event<>                   focused;
        Event<>                   focusLost;
        Event<uint32_t, uint32_t> resized;
        Event<uint32_t, uint32_t> moved;
        Event<>                   refresh;

        // user input
        Event<uint32_t>           keyPressed;
        Event<uint32_t>           keyReleased;
        Event<int32_t, int32_t>   mouseMove;
        Event<uint32_t, uint32_t> leftMouseButtonPressed;
        Event<uint32_t, uint32_t> leftMouseButtonReleased;
        Event<uint32_t, uint32_t> rightMouseButtonPressed;
        Event<uint32_t, uint32_t> rightMouseButtonReleased;
        Event<int32_t>            scroll;
    } events;

public:
    USING_CREATE (Window);

    using DrawCallback = std::function<void (bool& stopFlag)>;

    enum class Mode {
        Fullscreen,
        Windowed,
    };

    virtual ~Window () = default;

    virtual void  DoEventLoop (const DrawCallback&) = 0;
    virtual void* GetHandle () const                = 0;

    virtual std::vector<const char*> GetExtensions () const           = 0;
    virtual VkSurfaceKHR             GetSurface (VkInstance instance) = 0;

    virtual uint32_t GetWidth () const       = 0;
    virtual uint32_t GetHeight () const      = 0;
    virtual float    GetAspectRatio () const = 0;


    virtual void Show ()  = 0;
    virtual void Hide ()  = 0;
    virtual void Focus () = 0;

    virtual void ToggleFullscreen () = 0;
    virtual void SetWindowMode (Mode) {}
    virtual Mode GetWindowMode () { throw std::runtime_error ("unimplemeted"); }
};

#endif