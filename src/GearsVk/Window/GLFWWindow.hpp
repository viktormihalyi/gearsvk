#ifndef GLFWWINDOW_HPP
#define GLFWWINDOW_HPP

#include "GearsVkAPI.hpp"

#include "Window.hpp"

#include <optional>
#include <vector>

class GEARSVK_API GLFWWindowBase : public Window {
private:
    void*        window;
    VkSurfaceKHR surface;

protected:
    GLFWWindowBase (const std::vector<std::pair<int, int>>& hints);

public:
    USING_PTR_ABSTRACT (GLFWWindowBase);

    virtual ~GLFWWindowBase () override;

    void  DoEventLoop (const DrawCallback&) override;
    void* GetHandle () const override;

    std::vector<const char*> GetExtensions () const override;

    virtual uint32_t GetWidth () const override { return 800; }         // TODO
    virtual uint32_t GetHeight () const override { return 600; }        // TODO
    virtual float    GetAspectRatio () const override { return 1.33f; } // TODO
    virtual void     ToggleFullscreen () override {}

    virtual void Show () override;
    virtual void Hide () override;
    virtual void Focus () override;

    VkSurfaceKHR GetSurface (VkInstance instance) override;
};


class GEARSVK_API GLFWWindow : public GLFWWindowBase {
public:
    USING_PTR (GLFWWindow);
    GLFWWindow ();
    virtual ~GLFWWindow () = default;
};


class GEARSVK_API HiddenGLFWWindow : public GLFWWindowBase {
public:
    USING_PTR (HiddenGLFWWindow);
    HiddenGLFWWindow ();
    virtual ~HiddenGLFWWindow () = default;
};

#endif