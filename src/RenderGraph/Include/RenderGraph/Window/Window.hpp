#ifndef WINDOWB_HPP
#define WINDOWB_HPP

#include "RenderGraph/RenderGraphAPI.hpp"

#include "Utils/Event.hpp"
#include "Utils/Noncopyable.hpp"
#include <memory>

#include <functional>
#include <vector>

#include <vulkan/vulkan.h>

namespace GVK {

class GVK_RENDERER_API Window : public Noncopyable {
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
    using DrawCallback = std::function<void (bool& stopFlag)>;

    enum class Mode {
        Fullscreen,
        Windowed,
    };

    virtual ~Window () = default;

    virtual void  DoEventLoop (const DrawCallback&) = 0;
    virtual void* GetHandle () const                = 0;

    virtual VkSurfaceKHR GetSurface (VkInstance instance) = 0;

    virtual void PollEvents () = 0;

    virtual uint32_t GetWidth () const       = 0;
    virtual uint32_t GetHeight () const      = 0;
    virtual float    GetAspectRatio () const = 0;
    virtual double   GetRefreshRate () const   = 0;


    virtual void Show ()  = 0;
    virtual void Hide ()  = 0;
    virtual void Focus () = 0;
    virtual void Close () = 0;

    virtual void SetWindowMode (Mode) = 0;
    virtual Mode GetWindowMode ()     = 0;
};

} // namespace GVK

#endif