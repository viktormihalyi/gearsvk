#include "Surface.hpp"

#include "Assert.hpp"

#include <stdexcept>

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vulkan/vulkan_win32.h>

Surface::Surface (PlatformSelector, VkInstance instance, void* hwnd)
    : instance (instance)
    , handle (VK_NULL_HANDLE)
{
    VkWin32SurfaceCreateInfoKHR info = {};
    info.sType                       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    info.hinstance                   = GetModuleHandle (nullptr);
    info.hwnd                        = static_cast<HWND> (hwnd);

    if (GVK_ERROR (vkCreateWin32SurfaceKHR (instance, &info, nullptr, &handle) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create win32 surface");
    }
}

#endif
