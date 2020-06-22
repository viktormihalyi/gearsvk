#ifndef SDLWINDOW_HPP
#define SDLWINDOW_HPP

#include "GearsVkAPI.hpp"

#include "Window.hpp"

#include <optional>
#include <vector>

USING_PTR (SDLWindowBase);
class GEARSVK_API SDLWindowBase : public Window {
private:
    static uint32_t windowCount;

    void* window;

    uint32_t fullscreenWidth, fullscreenHeight;
    uint32_t width;
    uint32_t height;

    bool isFullscreen;

protected:
    SDLWindowBase (uint32_t flags);

public:
    virtual ~SDLWindowBase () override;

    void  DoEventLoop (const DrawCallback&) override;
    void* GetHandle () const override;

    std::vector<const char*> GetExtensions () const override;

    VkSurfaceKHR GetSurface (VkInstance instance) override;

    virtual uint32_t GetWidth () const override { return width; }
    virtual uint32_t GetHeight () const override { return height; }
    virtual float    GetAspectRatio () const override { return static_cast<float> (width) / height; }

    virtual void ToggleFullscreen ();
};


USING_PTR (SDLWindow);
class SDLWindow final : public SDLWindowBase {
public:
    USING_CREATE (SDLWindow);
    SDLWindow ();
};


USING_PTR (HiddenSDLWindow);
class HiddenSDLWindow final : public SDLWindowBase {
public:
    USING_CREATE (HiddenSDLWindow);
    HiddenSDLWindow ();
};

#endif