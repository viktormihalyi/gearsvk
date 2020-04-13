#include "GLFWWindow.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Assert.hpp"
#include "Timer.hpp"
#include <iostream>


GLFWWindowBase::GLFWWindowBase (const std::vector<std::pair<int, int>>& hints)
    : window (nullptr)
{
    // settings
    const bool useFullscreen = false;
    const bool hideMouse     = false;

    glfwInit ();

    ASSERT (glfwVulkanSupported () == GLFW_TRUE);

    glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint (GLFW_RESIZABLE, GLFW_TRUE);

    for (auto hint : hints) {
        glfwWindowHint (hint.first, hint.second);
    }

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
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        const char* keyName = glfwGetKeyName (key, 0);

        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose (window, GLFW_TRUE);
        }

        if (action == GLFW_PRESS) {
            self->events.keyPressed (key);
        } else if (action == GLFW_RELEASE) {
            self->events.keyReleased (key);
        }
    });


    glfwSetCursorPosCallback (glfwWindow, [] (GLFWwindow* window, double xpos, double ypos) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        self->events.mouseMove (xpos, ypos);
    });


    glfwSetMouseButtonCallback (glfwWindow, [] (GLFWwindow* window, int button, int action, int mods) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        double x, y;
        glfwGetCursorPos (window, &x, &y);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            self->events.leftMouseButtonPressed (x, y);
        } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            self->events.leftMouseButtonReleased (x, y);
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            self->events.rightMouseButtonPressed (x, y);
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
            self->events.rightMouseButtonReleased (x, y);
        }
    });


    glfwSetScrollCallback (glfwWindow, [] (GLFWwindow* window, double xoffset, double yoffset) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        self->events.scroll (yoffset);
    });

    glfwSetWindowSizeCallback (glfwWindow, [] (GLFWwindow* window, int width, int height) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        self->events.resized (width, height);
    });

    glfwSetWindowPosCallback (glfwWindow, [] (GLFWwindow* window, int xpos, int ypos) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        self->events.moved (xpos, ypos);
    });

    glfwSetWindowFocusCallback (glfwWindow, [] (GLFWwindow* window, int focused) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        if (focused) {
            self->events.focused ();
        } else {
            self->events.focusLost ();
        }
    });

    window = reinterpret_cast<void*> (glfwWindow);
}


GLFWWindowBase::~GLFWWindowBase ()
{
    glfwDestroyWindow (reinterpret_cast<GLFWwindow*> (window));
    glfwTerminate ();
}


void* GLFWWindowBase::GetHandle () const
{
    return window;
}


void GLFWWindowBase::DoEventLoop (const DrawCallback& drawCallback)
{
    if (ERROR (window == nullptr)) {
        return;
    }

    while (!glfwWindowShouldClose (reinterpret_cast<GLFWwindow*> (window))) {
        glfwPollEvents ();

        bool stop = false;

        drawCallback (stop);

        if (stop) {
            break;
        }
    }
}


std::vector<const char*> GLFWWindowBase::GetExtensions () const
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


VkSurfaceKHR GLFWWindowBase::CreateSurface (VkInstance instance) const
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    ASSERT (window != nullptr);

    VkResult result = glfwCreateWindowSurface (instance, reinterpret_cast<GLFWwindow*> (window), nullptr, &surface);
    if (result != VK_SUCCESS) {
        throw std::runtime_error ("Failed to create glfw surface");
    }

    return surface;
}


GLFWWindow::GLFWWindow ()
    : GLFWWindowBase ({})
{
}


HiddenGLFWWindow::HiddenGLFWWindow ()
    : GLFWWindowBase ({
          {GLFW_VISIBLE, GLFW_FALSE},
      })
{
}
