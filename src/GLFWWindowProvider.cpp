#include "GLFWWindowProvider.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Assert.hpp"
#include <iostream>


GLFWWindowProvider::GLFWWindowProvider ()
    : window (nullptr)
{
    glfwInit ();

    ASSERT (glfwVulkanSupported () == GLFW_TRUE);

    glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint (GLFW_RESIZABLE, GLFW_FALSE);
    // glfwWindowHint (GLFW_VISIBLE, GLFW_FALSE);

    glfwSwapInterval (0);


    int           monitorCount = 0;
    GLFWmonitor** monitors     = glfwGetMonitors (&monitorCount);
    for (int i = 0; i < monitorCount; ++i) {
        GLFWmonitor* monitor     = monitors[i];
        const char*  monitorName = glfwGetMonitorName (monitor);
        std::string  mo (monitorName);
    }

    window = reinterpret_cast<void*> (glfwCreateWindow (800, 600, "test", nullptr, nullptr));
    if (ERROR (window == nullptr)) {
        std::cerr << "failed to create window" << std::endl;
        exit (EXIT_FAILURE);
    }
}


GLFWWindowProvider::~GLFWWindowProvider ()
{
    glfwDestroyWindow (reinterpret_cast<GLFWwindow*> (window));
    glfwTerminate ();
}


void* GLFWWindowProvider::GetHandle () const
{
    return window;
}


void GLFWWindowProvider::DoEventLoop (const DrawCallback& drawCallback)
{
    if (ERROR (window == nullptr)) {
        return;
    }

    while (!glfwWindowShouldClose (reinterpret_cast<GLFWwindow*> (window))) {
        glfwPollEvents ();

        drawCallback ();

        glfwWaitEvents ();
    }
}


std::vector<const char*> GLFWWindowProvider::GetExtensions () const
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


VkSurfaceKHR GLFWWindowProvider::CreateSurface (VkInstance instance) const
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkResult result = glfwCreateWindowSurface (instance, reinterpret_cast<GLFWwindow*> (window), nullptr, &surface);
    if (result != VK_SUCCESS) {
        throw std::runtime_error ("Failed to create glfw surface");
    }

    return surface;
}
