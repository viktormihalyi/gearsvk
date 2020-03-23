#include "SDLWindowProvider.hpp"

#include <SDL.h>
#include <SDL_vulkan.h>

#include "Assert.hpp"
#include "Timer.hpp"

#include <iostream>


SDLWindowProvider::SDLWindowProvider ()
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
                               SDL_WINDOW_VULKAN);
}


SDLWindowProvider::~SDLWindowProvider ()
{
    SDL_Quit ();
}


void* SDLWindowProvider::GetHandle () const
{
    return window;
}


void SDLWindowProvider::DoEventLoop (const DrawCallback& drawCallback)
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
            drawCallback ();
        }
    }
}


std::vector<const char*> SDLWindowProvider::GetExtensions () const
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


VkSurfaceKHR SDLWindowProvider::CreateSurface (VkInstance instance) const
{
    ASSERT (window != nullptr);

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (!SDL_Vulkan_CreateSurface (reinterpret_cast<SDL_Window*> (window), instance, &surface)) {
        throw std::runtime_error ("sdl failed to create surface");
    }

    return surface;
}
