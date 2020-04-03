#ifndef GLFWWINDOW_HPP
#define GLFWWINDOW_HPP

#include "Window.hpp"

#include <optional>
#include <vector>

class GLFWWindow final : public Window {
private:
    void* window;

public:
    USING_PTR (GLFWWindow);

    GLFWWindow ();
    ~GLFWWindow () override;

    void  DoEventLoop (const DrawCallback&) override;
    void* GetHandle () const override;

    std::vector<const char*> GetExtensions () const override;

    VkSurfaceKHR CreateSurface (VkInstance instance) const override;
};

#endif