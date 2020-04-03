#include "SDLWindow.hpp"

#include <SDL.h>
#include <SDL_vulkan.h>

#include "Assert.hpp"
#include "Timer.hpp"

#include <iostream>


uint32_t SDLWindowBase::windowCount = 0;


SDLWindowBase::SDLWindowBase (uint32_t flags)
    : window (nullptr)
    , width (800)
    , height (600)
{
    if (ERROR (windowCount != 0)) {
        throw std::runtime_error ("TODO support multiple windows");
    }

    if (ERROR (SDL_Init (SDL_INIT_VIDEO) != 0)) {
        throw std::runtime_error ("sdl init failed");
    }

    window = SDL_CreateWindow ("vulkantest",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               width, height,
                               SDL_WINDOW_VULKAN | flags);

    ++windowCount;
}


SDLWindowBase::~SDLWindowBase ()
{
    SDL_Quit ();

    --windowCount;
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
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN: events.keyPressed (static_cast<uint32_t> (e.key.keysym.sym)); break;
                case SDL_KEYUP: events.keyReleased (static_cast<uint32_t> (e.key.keysym.sym)); break;
                case SDL_MOUSEMOTION: events.mouseMove (static_cast<uint32_t> (e.motion.x), static_cast<uint32_t> (e.motion.y)); break;
                case SDL_MOUSEBUTTONDOWN:
                    switch (e.button.button) {
                        case SDL_BUTTON_LEFT: events.leftMouseButtonPressed (static_cast<uint32_t> (e.button.x), static_cast<uint32_t> (e.button.y)); break;
                        case SDL_BUTTON_RIGHT: events.rightMouseButtonPressed (static_cast<uint32_t> (e.button.x), static_cast<uint32_t> (e.button.y)); break;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    switch (e.button.button) {
                        case SDL_BUTTON_LEFT: events.leftMouseButtonReleased (static_cast<uint32_t> (e.button.x), static_cast<uint32_t> (e.button.y)); break;
                        case SDL_BUTTON_RIGHT: events.rightMouseButtonReleased (static_cast<uint32_t> (e.button.x), static_cast<uint32_t> (e.button.y)); break;
                    }
                    break;
                case SDL_MOUSEWHEEL: events.scroll (static_cast<int32_t> (e.wheel.y)); break;
                case SDL_WINDOWEVENT:
                    switch (e.window.event) {
                        case SDL_WINDOWEVENT_SHOWN:
                            events.shown ();
                            break;
                        case SDL_WINDOWEVENT_HIDDEN:
                            events.hidden ();
                            break;
                        case SDL_WINDOWEVENT_MOVED:
                            events.moved (static_cast<uint32_t> (e.window.data1), static_cast<uint32_t> (e.window.data2));
                            break;
                        case SDL_WINDOWEVENT_RESIZED:
                            events.resized (static_cast<uint32_t> (e.window.data1), static_cast<uint32_t> (e.window.data2));
                            break;
                        case SDL_WINDOWEVENT_FOCUS_GAINED:
                            events.focused ();
                            break;
                        case SDL_WINDOWEVENT_FOCUS_LOST:
                            events.focusLost ();
                            break;
                        case SDL_WINDOWEVENT_CLOSE:
                            events.closed ();
                            break;
                    }
                    break;
            }
        }

        if (quit) {
            break;
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
