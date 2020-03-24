#include "GLFWWindow.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Assert.hpp"
#include "Timer.hpp"
#include <iostream>


GLFWWindow::GLFWWindow ()
    : window (nullptr)
{
    // settings
    const bool useFullscreen = false;
    const bool hideMouse     = false;

    glfwInit ();

    ASSERT (glfwVulkanSupported () == GLFW_TRUE);

    glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint (GLFW_RESIZABLE, GLFW_FALSE);
    // glfwWindowHint (GLFW_VISIBLE, GLFW_FALSE);

    // monitor settings

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor ();
    glfwSetMonitorCallback ([] (GLFWmonitor* monitor, int event) {
        if (event == GLFW_CONNECTED) {
            std::cout << "connected" << std::endl;
        } else if (event == GLFW_DISCONNECTED) {
            std::cout << "disconnected" << std::endl;
        }
    });

    int width_mm, height_mm;
    glfwGetMonitorPhysicalSize (primaryMonitor, &width_mm, &height_mm);

    float xscale, yscale;
    glfwGetMonitorContentScale (primaryMonitor, &xscale, &yscale);

    int virtual_xpos, virtual_ypos;
    glfwGetMonitorPos (primaryMonitor, &virtual_xpos, &virtual_ypos);

    int xpos, ypos, width, height;
    glfwGetMonitorWorkarea (primaryMonitor, &xpos, &ypos, &width, &height);

    const GLFWgammaramp* gammaRamp = glfwGetGammaRamp (primaryMonitor);

    int          windowWidth  = 800;
    int          windowHeight = 600;
    GLFWmonitor* usedMonitor  = nullptr;

    if (useFullscreen) {
        const GLFWvidmode* mode = glfwGetVideoMode (primaryMonitor);
        windowWidth             = mode->width;
        windowHeight            = mode->height;
        usedMonitor             = primaryMonitor;
    }

    GLFWwindow* glfwWindow = glfwCreateWindow (windowWidth, windowHeight, "test", usedMonitor, nullptr);
    if (ERROR (glfwWindow == nullptr)) {
        throw std::runtime_error ("failed to create window");
    }

    // window settings

    glfwSetWindowUserPointer (glfwWindow, this);
    std::cout << std::boolalpha << "glfwRawMouseMotionSupported is " << (glfwRawMouseMotionSupported () == GLFW_TRUE) << std::endl;
    if (hideMouse) {
        glfwSetInputMode (glfwWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // GLFW_CURSOR_DISABLED
    }

    // callbacks

    glfwSetKeyCallback (glfwWindow, [] (GLFWwindow* window, int key, int scancode, int action, int mods) {
        GLFWWindow* self = static_cast<GLFWWindow*> (glfwGetWindowUserPointer (window));

        const char* keyName = glfwGetKeyName (key, 0);
        std::cout << "glfwSetKeyCallback, key: " << key << " (" << (keyName != nullptr ? keyName : "???") << "), scancode: " << scancode << ", action: " << action << ", mods: " << mods << std::endl;

        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose (window, GLFW_TRUE);
        }
    });


    glfwSetCursorPosCallback (glfwWindow, [] (GLFWwindow* window, double xpos, double ypos) {
        GLFWWindow* self = static_cast<GLFWWindow*> (glfwGetWindowUserPointer (window));

        std::cout << "glfwSetCursorPosCallback, xpos: " << xpos << ", ypos: " << ypos << std::endl;
    });


    glfwSetMouseButtonCallback (glfwWindow, [] (GLFWwindow* window, int button, int action, int mods) {
        GLFWWindow* self = static_cast<GLFWWindow*> (glfwGetWindowUserPointer (window));

        std::cout << "glfwSetMouseButtonCallback, button: " << button << ", action: " << action << ", mods: " << mods << std::endl;
    });


    glfwSetScrollCallback (glfwWindow, [] (GLFWwindow* window, double xoffset, double yoffset) {
        GLFWWindow* self = static_cast<GLFWWindow*> (glfwGetWindowUserPointer (window));

        std::cout << "glfwSetScrollCallback, xoffset: " << xoffset << ", yoffset: " << yoffset << std::endl;
    });


    window = reinterpret_cast<void*> (glfwWindow);
}


GLFWWindow::~GLFWWindow ()
{
    glfwDestroyWindow (reinterpret_cast<GLFWwindow*> (window));
    glfwTerminate ();
}


void* GLFWWindow::GetHandle () const
{
    return window;
}


void GLFWWindow::DoEventLoop (const DrawCallback& drawCallback)
{
    if (ERROR (window == nullptr)) {
        return;
    }

    while (!glfwWindowShouldClose (reinterpret_cast<GLFWwindow*> (window))) {
        glfwPollEvents ();

        drawCallback ();
    }
}


std::vector<const char*> GLFWWindow::GetExtensions () const
{
    uint32_t     glfwExtensionCount = 0;
    const char** glfwExtensions     = glfwGetRequiredInstanceExtensions (&glfwExtensionCount);
    ASSERT (glfwExtensionCount != 0);

    std::vector<const char*> result;
    for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
        result.push_back (glfwExtensions[i]);
    }
    return result;
}


VkSurfaceKHR GLFWWindow::CreateSurface (VkInstance instance) const
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    ASSERT (window != nullptr);

    VkResult result = glfwCreateWindowSurface (instance, reinterpret_cast<GLFWwindow*> (window), nullptr, &surface);
    if (result != VK_SUCCESS) {
        throw std::runtime_error ("Failed to create glfw surface");
    }

    return surface;
}
