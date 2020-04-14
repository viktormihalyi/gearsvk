#ifndef GLFWWINDOW_HPP
#define GLFWWINDOW_HPP

#include "Window.hpp"

#include <optional>
#include <vector>

class GLFWWindowBase : public Window {
private:
    void* window;

protected:
    GLFWWindowBase (const std::vector<std::pair<int, int>>& hints);

public:
    USING_PTR_ABSTRACT (GLFWWindowBase);

    virtual ~GLFWWindowBase () override;

    void  DoEventLoop (const DrawCallback&) override;
    void* GetHandle () const override;

    std::vector<const char*> GetExtensions () const override;

    virtual uint32_t GetWidth () const override { return 800; }
    virtual uint32_t GetHeight () const override { return 600; }
    virtual float    GetAspectRatio () const override { return 1.33; }
    virtual void     ToggleFullscreen () override {}

    virtual void Show () override;
    virtual void Hide () override;
    virtual void Focus () override;

    VkSurfaceKHR CreateSurface (VkInstance instance) const override;
};


class GLFWWindow : public GLFWWindowBase {
public:
    USING_PTR (GLFWWindow);
    GLFWWindow ();
    virtual ~GLFWWindow () = default;
};


class HiddenGLFWWindow : public GLFWWindowBase {
public:
    USING_PTR (HiddenGLFWWindow);
    HiddenGLFWWindow ();
    virtual ~HiddenGLFWWindow () = default;
};

#endif