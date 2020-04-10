#ifndef GRAPHSETTINGS_HPP
#define GRAPHSETTINGS_HPP

#include <vulkan/vulkan.h>

struct GraphSettings {
    Device*       device;
    VkQueue       queue;
    VkCommandPool commandPool;
    uint32_t      framesInFlight;
    uint32_t      width;
    uint32_t      height;

    GraphSettings (Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t framesInFlight, uint32_t width, uint32_t height)
        : device (&device)
        , queue (queue)
        , commandPool (commandPool)
        , framesInFlight (framesInFlight)
        , width (width)
        , height (height)
    {
    }

    GraphSettings (Device& device, VkQueue queue, VkCommandPool commandPool, const Swapchain& swapchain)
        : GraphSettings (device, queue, commandPool, swapchain.GetImageCount (), swapchain.GetWidth (), swapchain.GetHeight ())
    {
    }

    Device& GetDevice () const { return *device; }
};

#endif