#ifndef GLFWWINDOWPROVIDER_HPP
#define GLFWWINDOWPROVIDER_HPP

#include "WindowProvider.hpp"

#include <optional>
#include <vector>

class GLFWWindowProvider final : public WindowProvider {
private:
    void* window;

public:
    GLFWWindowProvider ();
    ~GLFWWindowProvider () override;

    void  DoEventLoop (const DrawCallback&) override;
    void* GetHandle () const override;

    std::vector<const char*> GetExtensions () const override;

    VkSurfaceKHR CreateSurface (VkInstance instance) const override;
};

#endif