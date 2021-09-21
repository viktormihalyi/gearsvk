#ifndef GLFWWINDOW_HPP
#define GLFWWINDOW_HPP

#include "RenderGraph/RenderGraphAPI.hpp"

#include "Window.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace RG {

class GVK_RENDERER_API GLFWWindowBase : public Window {
private:
    struct Impl;
    std::unique_ptr<Impl> impl;

protected:
    GLFWWindowBase (const std::vector<std::pair<int, int>>& hints, bool useFullscreen, bool hideMouse);

public:
    virtual ~GLFWWindowBase () override;

    void  DoEventLoop (const DrawCallback&) override;
    void* GetHandle () const override;

    virtual uint32_t GetWidth () const override;
    virtual uint32_t GetHeight () const override;
    virtual float    GetAspectRatio () const override;
    virtual double   GetRefreshRate () const override;

    virtual void SetWindowMode (Mode) override;
    virtual Mode GetWindowMode () override;

    virtual void Show () override;
    virtual void Hide () override;
    virtual void Focus () override;

    virtual void SetTitle (const std::string&) override;

    virtual VkSurfaceKHR GetSurface (VkInstance instance) override;
    virtual void         PollEvents () override;
    virtual void         Close () override;
};


class GVK_RENDERER_API GLFWWindow : public GLFWWindowBase {
public:
    GLFWWindow ();
    virtual ~GLFWWindow () = default;
};


class GVK_RENDERER_API FullscreenGLFWWindow : public GLFWWindowBase {
public:
    FullscreenGLFWWindow ();
    virtual ~FullscreenGLFWWindow () = default;
};

class GVK_RENDERER_API HiddenGLFWWindow : public GLFWWindowBase {
public:
    HiddenGLFWWindow ();
    virtual ~HiddenGLFWWindow () = default;
};


GVK_RENDERER_API
std::vector<const char*> GetGLFWInstanceExtensions ();

} // namespace RG

#endif