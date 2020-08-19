#include "GLFWWindow.hpp"

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Assert.hpp"
#include "Timer.hpp"
#include <iostream>


struct GLFWInitializer {
private:
    bool initialized;

public:
    GLFWInitializer ()
        : initialized (false)
    {
    }

    void EnsureInitialized ()
    {
        if (!initialized) {
            initialized = true;
            int result  = glfwInit ();
            GVK_ASSERT (result == GLFW_TRUE);
        }
    }

    ~GLFWInitializer ()
    {
        if (initialized) {
            glfwTerminate ();
        }
    }
};


static GLFWInitializer globalGLFWInitializer;


constexpr uint32_t InitialWindowWidth  = 800;
constexpr uint32_t InitialWindowHeight = 600;


GLFWWindowBase::GLFWWindowBase (const std::vector<std::pair<int, int>>& hints)
    : window (nullptr)
    , surface (VK_NULL_HANDLE)
    , width (InitialWindowWidth)
    , height (InitialWindowHeight)
{
    globalGLFWInitializer.EnsureInitialized ();

    // settings
    const bool useFullscreen = false;
    const bool hideMouse     = false;

    GVK_ASSERT (glfwVulkanSupported () == GLFW_TRUE);

    glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint (GLFW_RESIZABLE, GLFW_TRUE);

    for (auto hint : hints) {
        glfwWindowHint (hint.first, hint.second);
    }

    // monitor settings

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor ();
    glfwSetMonitorCallback ([] (GLFWmonitor* monitor, int event) {
        // TODO
    });

    int width_mm, height_mm;
    glfwGetMonitorPhysicalSize (primaryMonitor, &width_mm, &height_mm);

    float xscale, yscale;
    glfwGetMonitorContentScale (primaryMonitor, &xscale, &yscale);
    GVK_ASSERT (xscale == 1.f && yscale == 1.f); // TODO handle different content scale (window size != framebuffer size)

    int virtual_xpos, virtual_ypos;
    glfwGetMonitorPos (primaryMonitor, &virtual_xpos, &virtual_ypos);

    //int xpos, ypos, width, height;
    //glfwGetMonitorWorkarea (primaryMonitor, &xpos, &ypos, &width, &height);

    const GLFWgammaramp* gammaRamp = glfwGetGammaRamp (primaryMonitor);

    GLFWmonitor* usedMonitor = nullptr;

    if (useFullscreen) {
        const GLFWvidmode* mode = glfwGetVideoMode (primaryMonitor);
        width                   = mode->width;
        height                  = mode->height;
        usedMonitor             = primaryMonitor;
    }

    GLFWwindow* glfwWindow = glfwCreateWindow (width, height, "test", usedMonitor, nullptr);
    if (GVK_ERROR (glfwWindow == nullptr)) {
        throw std::runtime_error ("failed to create window");
    }

    // window settings

    glfwSetWindowUserPointer (glfwWindow, this);
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
            self->events.leftMouseButtonPressed (static_cast<uint32_t> (x), static_cast<uint32_t> (y));
        } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            self->events.leftMouseButtonReleased (static_cast<uint32_t> (x), static_cast<uint32_t> (y));
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            self->events.rightMouseButtonPressed (static_cast<uint32_t> (x), static_cast<uint32_t> (y));
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
            self->events.rightMouseButtonReleased (static_cast<uint32_t> (x), static_cast<uint32_t> (y));
        }
    });


    glfwSetScrollCallback (glfwWindow, [] (GLFWwindow* window, double /* xoffset */, double yoffset) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        self->events.scroll (yoffset);
    });

    glfwSetWindowSizeCallback (glfwWindow, [] (GLFWwindow* window, int width, int height) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        self->width  = width;
        self->height = height;

        std::cout << "window resized" << std::endl;
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

    glfwSetWindowRefreshCallback (glfwWindow, [] (GLFWwindow* window) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));
        self->events.refresh ();
    });

    glfwSetFramebufferSizeCallback (glfwWindow, [] (GLFWwindow* window, int width, int height) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        std::cout << "framebuffer resized" << std::endl;
    });

    window = reinterpret_cast<void*> (glfwWindow);
}


GLFWWindowBase::~GLFWWindowBase ()
{
    surface = VK_NULL_HANDLE;
    glfwDestroyWindow (reinterpret_cast<GLFWwindow*> (window));
}


void* GLFWWindowBase::GetHandle () const
{
    return window;
}


uint32_t GLFWWindowBase::GetWidth () const
{
    return width;
}


uint32_t GLFWWindowBase::GetHeight () const
{
    return height;
}


float GLFWWindowBase::GetAspectRatio () const
{
    return static_cast<float> (width) / height;
}


void GLFWWindowBase::DoEventLoop (const DrawCallback& drawCallback)
{
    if (GVK_ERROR (window == nullptr)) {
        return;
    }

    while (!glfwWindowShouldClose (reinterpret_cast<GLFWwindow*> (window))) {
        glfwPollEvents ();

        bool stop = false;

        drawCallback (stop);

        if (stop) {
            glfwSetWindowShouldClose (reinterpret_cast<GLFWwindow*> (window), GLFW_TRUE);
        }
    }
}


std::vector<const char*> GLFWWindowBase::GetExtensions () const
{
    uint32_t     glfwExtensionCount = 0;
    const char** glfwExtensions     = glfwGetRequiredInstanceExtensions (&glfwExtensionCount);
    GVK_ASSERT (glfwExtensionCount != 0);

    std::vector<const char*> result;
    for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
        result.push_back (glfwExtensions[i]);
    }
    return result;
}


VkSurfaceKHR GLFWWindowBase::GetSurface (VkInstance instance)
{
    GVK_ASSERT (window != nullptr);

    if (surface != VK_NULL_HANDLE) {
        return surface;
    }

    VkResult result = glfwCreateWindowSurface (instance, reinterpret_cast<GLFWwindow*> (window), nullptr, &surface);
    if (result != VK_SUCCESS) {
        throw std::runtime_error ("Failed to create glfw surface");
    }

    return surface;
}


void GLFWWindowBase::Hide ()
{
    glfwHideWindow (reinterpret_cast<GLFWwindow*> (window));
}


void GLFWWindowBase::Show ()
{
    glfwShowWindow (reinterpret_cast<GLFWwindow*> (window));
}


void GLFWWindowBase::Focus ()
{
    glfwFocusWindow (reinterpret_cast<GLFWwindow*> (window));
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
