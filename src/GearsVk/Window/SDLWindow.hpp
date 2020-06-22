#ifndef SDLWINDOW_HPP
#define SDLWINDOW_HPP

#include "GearsVkAPI.hpp"

#include "Window.hpp"

#include <optional>
#include <vector>

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
    USING_PTR_ABSTRACT (SDLWindowBase);

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


USING_PTR_2 (SDLWindow);
class SDLWindow final : public SDLWindowBase {
public:
    USING_PTR (SDLWindow);
    SDLWindow ();
};


USING_PTR_2 (HiddenSDLWindow);
class HiddenSDLWindow final : public SDLWindowBase {
public:
    USING_PTR (HiddenSDLWindow);
    HiddenSDLWindow ();
};

#endif