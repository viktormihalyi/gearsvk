#ifndef GLFWWINDOW_HPP
#define GLFWWINDOW_HPP

#include "GearsVkAPI.hpp"

#include "Window.hpp"

#include <memory>
#include <optional>
#include <vector>


USING_PTR (GLFWWindowBase);
class GEARSVK_API GLFWWindowBase : public Window {
private:
    struct Impl;
    std::unique_ptr<Impl> impl;

protected:
    GLFWWindowBase (const std::vector<std::pair<int, int>>& hints);

public:
    virtual ~GLFWWindowBase () override;

    void  DoEventLoop (const DrawCallback&) override;
    void* GetHandle () const override;

    virtual uint32_t GetWidth () const override;
    virtual uint32_t GetHeight () const override;
    virtual float    GetAspectRatio () const override;

    virtual void SetWindowMode (Mode) override;
    virtual Mode GetWindowMode () override;

    virtual void Show () override;
    virtual void Hide () override;
    virtual void Focus () override;

    virtual VkSurfaceKHR GetSurface (VkInstance instance) override;
    virtual void         PollEvents () override;
    virtual void         Close () override;
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