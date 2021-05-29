#include "GLFWWindow.hpp"

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Assert.hpp"
#include "Timer.hpp"
#include <iostream>

namespace GVK {


static void error_callback (int error, const char *description)
{
    std::cout << "GLFW ERROR " << error << ": " << description << std::endl;
}


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
            if (GVK_ERROR (result != GLFW_TRUE)) {
                std::terminate ();
            }
            
            glfwSetErrorCallback (error_callback);
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


struct GLFWWindowBase::Impl {
    GLFWwindow*  window;
    VkSurfaceKHR surface;

    uint32_t width;
    uint32_t height;
    uint32_t refreshRate;

    uint32_t widthWindowed;
    uint32_t heightWindowed;
    int32_t  posXWindowed;
    int32_t  posYWindowed;

    Window::Mode mode;

    Impl ()
        : window (nullptr)
        , surface (VK_NULL_HANDLE)

        , width (0)
        , height (0)
        , refreshRate (0)

        , widthWindowed (0)
        , heightWindowed (0)
        , posXWindowed (0)
        , posYWindowed (0)
        , mode (Window::Mode::Windowed)
    {
    }
};


GLFWWindowBase::GLFWWindowBase (const std::vector<std::pair<int, int>>& hints)
    : impl (std::make_unique<Impl> ())
{
    impl->width  = InitialWindowWidth;
    impl->height = InitialWindowHeight;

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

    //const GLFWgammaramp* gammaRamp = glfwGetGammaRamp (primaryMonitor);

    GLFWmonitor* usedMonitor = nullptr;

    const GLFWvidmode* mode = glfwGetVideoMode (primaryMonitor);
    impl->refreshRate       = mode->refreshRate;

    if (useFullscreen) {
        impl->width  = mode->width;
        impl->height = mode->height;
        usedMonitor  = primaryMonitor;
    }

    impl->window = glfwCreateWindow (impl->width, impl->height, "test", usedMonitor, nullptr);
    if (GVK_ERROR (impl->window == nullptr)) {
        throw std::runtime_error ("failed to create window");
    }

    // window settings

    glfwSetWindowUserPointer (impl->window, this);
    if (hideMouse) {
        glfwSetInputMode (impl->window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // GLFW_CURSOR_DISABLED
    }

    // callbacks

    glfwSetKeyCallback (impl->window, [] (GLFWwindow* window, int key, int scancode, int action, int mods) {
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


    glfwSetCursorPosCallback (impl->window, [] (GLFWwindow* window, double xpos, double ypos) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        self->events.mouseMove (xpos, ypos);
    });


    glfwSetMouseButtonCallback (impl->window, [] (GLFWwindow* window, int button, int action, int mods) {
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


    glfwSetScrollCallback (impl->window, [] (GLFWwindow* window, double /* xoffset */, double yoffset) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        self->events.scroll (yoffset);
    });

    glfwSetWindowSizeCallback (impl->window, [] (GLFWwindow* window, int width, int height) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        self->impl->width  = width;
        self->impl->height = height;

        std::cout << "window resized to (" << self->impl->width << ", " << self->impl->height << ")" << std::endl;
        self->events.resized (width, height);
    });

    glfwSetWindowPosCallback (impl->window, [] (GLFWwindow* window, int xpos, int ypos) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        self->events.moved (xpos, ypos);
    });

    glfwSetWindowFocusCallback (impl->window, [] (GLFWwindow* window, int focused) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        if (focused) {
            std::cout << "window focus gained " << std::endl;
            self->events.focused ();
        } else {
            std::cout << "window focus lost " << std::endl;
            self->events.focusLost ();
        }
    });

    glfwSetWindowRefreshCallback (impl->window, [] (GLFWwindow* window) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));
        self->events.refresh ();
    });

    glfwSetFramebufferSizeCallback (impl->window, [] (GLFWwindow* window, int width, int height) {
        GLFWWindowBase* self = static_cast<GLFWWindowBase*> (glfwGetWindowUserPointer (window));

        std::cout << "framebuffer resized" << std::endl;
    });
}


GLFWWindowBase::~GLFWWindowBase ()
{
    impl->surface = VK_NULL_HANDLE;
    glfwDestroyWindow (impl->window);
    impl->window = nullptr;
}


void* GLFWWindowBase::GetHandle () const
{
    return impl->window;
}


uint32_t GLFWWindowBase::GetWidth () const
{
    return impl->width;
}


uint32_t GLFWWindowBase::GetHeight () const
{
    return impl->height;
}


float GLFWWindowBase::GetAspectRatio () const
{
    return static_cast<float> (impl->width) / impl->height;
}


double GLFWWindowBase::GetRefreshRate () const
{
    return impl->refreshRate;
}


void GLFWWindowBase::PollEvents ()
{
    glfwPollEvents ();
    if (glfwWindowShouldClose (impl->window)) {
        throw std::runtime_error ("window closing");
    }
}


void GLFWWindowBase::Close ()
{
    glfwSetWindowShouldClose (impl->window, GLFW_TRUE);
}


void GLFWWindowBase::DoEventLoop (const DrawCallback& drawCallback)
{
    if (GVK_ERROR (impl->window == nullptr)) {
        return;
    }

    while (!glfwWindowShouldClose (impl->window)) {
        glfwPollEvents ();

        bool stop = false;

        drawCallback (stop);

        if (stop) {
            glfwSetWindowShouldClose (impl->window, GLFW_TRUE);
        }
    }
}


std::vector<const char*> Window::GetExtensions ()
{
    globalGLFWInitializer.EnsureInitialized ();

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
    GVK_ASSERT (impl->window != nullptr);

    if (impl->surface != VK_NULL_HANDLE) {
        return impl->surface;
    }

    VkResult result = glfwCreateWindowSurface (instance, impl->window, nullptr, &impl->surface);
    if (GVK_ERROR (result != VK_SUCCESS)) {
        throw std::runtime_error ("Failed to create glfw surface");
    }

    return impl->surface;
}


void GLFWWindowBase::Hide ()
{
    glfwHideWindow (impl->window);
}


void GLFWWindowBase::Show ()
{
    glfwShowWindow (impl->window);
}


void GLFWWindowBase::Focus ()
{
    glfwFocusWindow (impl->window);
}


void GLFWWindowBase::SetWindowMode (Mode mode)
{
    GLFWmonitor*       primaryMonitor     = glfwGetPrimaryMonitor ();
    const GLFWvidmode* primaryMonitorMode = glfwGetVideoMode (primaryMonitor);

    if (mode != Window::Mode::Windowed) {
        glfwGetWindowPos (impl->window, &impl->posXWindowed, &impl->posYWindowed);
    }

    if (mode == Window::Mode::Fullscreen) {
        glfwSetWindowMonitor (impl->window, primaryMonitor, 0, 0, primaryMonitorMode->width, primaryMonitorMode->height, primaryMonitorMode->refreshRate);
    } else if (mode == Window::Mode::Windowed) {
        glfwSetWindowMonitor (impl->window, nullptr, 0, 0, 800, 600, primaryMonitorMode->refreshRate);
        glfwSetWindowPos (impl->window, impl->posXWindowed, impl->posYWindowed);
    } else {
        GVK_BREAK ("unexpected window mode type");
    }

    impl->mode = mode;
}


Window::Mode GLFWWindowBase::GetWindowMode ()
{
    return impl->mode;
}


GLFWWindow::GLFWWindow ()
    : GLFWWindowBase ({})
{
}


HiddenGLFWWindow::HiddenGLFWWindow ()
    : GLFWWindowBase ({
          { GLFW_VISIBLE, GLFW_FALSE },
      })
{
}


} // namespace GVK