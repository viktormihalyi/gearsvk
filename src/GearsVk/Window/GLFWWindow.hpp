#ifndef GLFWWINDOW_HPP
#define GLFWWINDOW_HPP

#include "GearsVkAPI.hpp"

#include "Window.hpp"

#include <optional>
#include <vector>

USING_PTR (GLFWWindowBase);
class GEARSVK_API GLFWWindowBase : public Window {
private:
    uint32_t     width;
    uint32_t     height;
    void*        window;
    VkSurfaceKHR surface;

protected:
    GLFWWindowBase (const std::vector<std::pair<int, int>>& hints);

public:
    virtual ~GLFWWindowBase () override;

    void  DoEventLoop (const DrawCallback&) override;
    void* GetHandle () const override;

    std::vector<const char*> GetExtensions () const override;

    virtual uint32_t GetWidth () const override;
    virtual uint32_t GetHeight () const override;
    virtual float    GetAspectRatio () const override;
    virtual void     ToggleFullscreen () override {}

    virtual void Show () override;
    virtual void Hide () override;
    virtual void Focus () override;

    VkSurfaceKHR GetSurface (VkInstance instance) override;
};


USING_PTR (GLFWWindow);
class GEARSVK_API GLFWWindow : public GLFWWindowBase {
public:
    USING_CREATE (GLFWWindow);
    GLFWWindow ();
    virtual ~GLFWWindow () = default;
};


USING_PTR (HiddenGLFWWindow);
class GEARSVK_API HiddenGLFWWindow : public GLFWWindowBase {
public:
    USING_CREATE (HiddenGLFWWindow);
    HiddenGLFWWindow ();
    virtual ~HiddenGLFWWindow () = default;
};

#endif