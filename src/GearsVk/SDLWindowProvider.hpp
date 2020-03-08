#ifndef SDLWINDOWPROVIDER_HPP
#define SDLWINDOWPROVIDER_HPP

#include "WindowProvider.hpp"

#include <optional>
#include <vector>

class SDLWindowProvider final : public WindowProvider {
private:
    void* window;

public:
    USING_PTR (SDLWindowProvider);

    SDLWindowProvider ();
    ~SDLWindowProvider () override;

    void  DoEventLoop (const DrawCallback&) override;
    void* GetHandle () const override;

    std::vector<const char*> GetExtensions () const override;

    VkSurfaceKHR CreateSurface (VkInstance instance) const override;
};

#endif