#ifndef SDLWINDOWPROVIDER_HPP
#define SDLWINDOWPROVIDER_HPP

#include "WindowProvider.hpp"

#include <optional>
#include <vector>

class SDLWindowProvider final : public WindowProvider {
private:
    void*    window;
    uint32_t width;
    uint32_t height;

public:
    USING_PTR (SDLWindowProvider);

    SDLWindowProvider ();
    ~SDLWindowProvider () override;

    void  DoEventLoop (const DrawCallback&) override;
    void* GetHandle () const override;

    std::vector<const char*> GetExtensions () const override;

    VkSurfaceKHR CreateSurface (VkInstance instance) const override;

    virtual uint32_t GetWidth () const override { return width; }
    virtual uint32_t GetHeight () const override { return height; }
};

#endif