#ifndef GRAPHSETTINGS_HPP
#define GRAPHSETTINGS_HPP

#include <vulkan/vulkan.h>

namespace RG {

struct GraphSettings {
    const DeviceExtra* device;
    uint32_t           framesInFlight;

    // TODO remove
    uint32_t width;
    uint32_t height;

    GraphSettings (const DeviceExtra& device, VkQueue queue, VkCommandPool commandPool, uint32_t framesInFlight, uint32_t width, uint32_t height)
        : device (&device)
        , framesInFlight (framesInFlight)
        , width (width)
        , height (height)
    {
    }

    GraphSettings (const DeviceExtra& device, uint32_t framesInFlight, uint32_t width, uint32_t height)
        : GraphSettings (device, device.GetGraphicsQueue (), device.GetCommandPool (), framesInFlight, width, height)
    {
    }
    GraphSettings (const DeviceExtra& device, const Swapchain& swapchain)
        : GraphSettings (device, device.GetGraphicsQueue (), device.GetCommandPool (), swapchain.GetImageCount (), swapchain.GetWidth (), swapchain.GetHeight ())
    {
    }


    GraphSettings ()
        : device (nullptr)
        , framesInFlight (0)
        , width (0)
        , height (0)
    {
    }

    const DeviceExtra& GetDevice () const { return *device; }
    const Queue&       GetGrahpicsQueue () const { return device->GetGraphicsQueue (); }
    const CommandPool& GetCommandPool () const { return device->GetCommandPool (); }
};

} // namespace RG

#endif