#ifndef SDLWINDOW_HPP
#define SDLWINDOW_HPP

#include "Window.hpp"

#include <optional>
#include <vector>

class SDLWindowBase : public Window {
private:
    static uint32_t windowCount;

    void*    window;
    uint32_t width;
    uint32_t height;


protected:
    SDLWindowBase (uint32_t flags);

public:
    USING_PTR_ABSTRACT (SDLWindowBase);

    virtual ~SDLWindowBase () override;

    void  DoEventLoop (const DrawCallback&) override;
    void* GetHandle () const override;

    std::vector<const char*> GetExtensions () const override;

    VkSurfaceKHR CreateSurface (VkInstance instance) const override;

    virtual uint32_t GetWidth () const override { return width; }
    virtual uint32_t GetHeight () const override { return height; }
};


class SDLWindow final : public SDLWindowBase {
public:
    USING_PTR (SDLWindow);
    SDLWindow ();
};


class HiddenSDLWindow final : public SDLWindowBase {
public:
    USING_PTR (HiddenSDLWindow);
    HiddenSDLWindow ();
};

#endif