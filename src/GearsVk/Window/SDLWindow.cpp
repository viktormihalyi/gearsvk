#include "SDLWindow.hpp"

#include <SDL.h>
#include <SDL_vulkan.h>

#include "Assert.hpp"
#include "Timer.hpp"

#include <iostream>


SDLWindowBase::SDLWindowBase (uint32_t flags)
    : window (nullptr)
    , width (800)
    , height (600)
{
    if (ERROR (SDL_Init (SDL_INIT_VIDEO) != 0)) {
        throw std::runtime_error ("sdl init failed");
    }

    window = SDL_CreateWindow ("vulkantest",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               width, height,
                               SDL_WINDOW_VULKAN | flags);

    events.created.Notify ();
}


SDLWindowBase::~SDLWindowBase ()
{
    SDL_Quit ();
    events.destroyed.Notify ();
}

void* SDLWindowBase::GetHandle () const
{
    return window;
}


void SDLWindowBase::DoEventLoop (const DrawCallback& drawCallback)
{
    ASSERT (window != nullptr);

    bool      quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent (&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
                break;
            }
        }

        bool stop = false;

        drawCallback (stop);

        if (stop) {
            quit = true;
            break;
        }
    }
}


std::vector<const char*> SDLWindowBase::GetExtensions () const
{
    ASSERT (window != nullptr);

    unsigned int count;
    if (ERROR (!SDL_Vulkan_GetInstanceExtensions (reinterpret_cast<SDL_Window*> (window), &count, nullptr))) {
        throw std::runtime_error ("sdl failed to get instance extensions");
    }

    std::vector<const char*> extensions (count);
    if (ERROR (!SDL_Vulkan_GetInstanceExtensions (reinterpret_cast<SDL_Window*> (window), &count, extensions.data ()))) {
        throw std::runtime_error ("sdl failed to get instance extensions");
    }

    return extensions;
}


VkSurfaceKHR SDLWindowBase::CreateSurface (VkInstance instance) const
{
    ASSERT (window != nullptr);

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (!SDL_Vulkan_CreateSurface (reinterpret_cast<SDL_Window*> (window), instance, &surface)) {
        throw std::runtime_error ("sdl failed to create surface");
    }

    return surface;
}


SDLWindow::SDLWindow ()
    : SDLWindowBase (0)
{
}

HiddenSDLWindow::HiddenSDLWindow ()
    : SDLWindowBase (SDL_WINDOW_HIDDEN)
{
}
