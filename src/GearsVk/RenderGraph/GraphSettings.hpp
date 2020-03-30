#ifndef GRAPHSETTINGS_HPP
#define GRAPHSETTINGS_HPP

#include <vulkan/vulkan.h>

struct GraphSettings {
    const Device&       device;
    const VkQueue       queue;
    const VkCommandPool commandPool;
    const uint32_t      framesInFlight;
    const uint32_t      width;
    const uint32_t      height;

    GraphSettings (Device& device, VkQueue queue, VkCommandPool commandPool, uint32_t framesInFlight, uint32_t width, uint32_t height)
        : device (device)
        , queue (queue)
        , commandPool (commandPool)
        , framesInFlight (framesInFlight)
        , width (width)
        , height (height)
    {
    }

    GraphSettings (Device& device, VkQueue queue, VkCommandPool commandPool, const Swapchain& swapchain, uint32_t framesInFlight)
        : GraphSettings (device, queue, commandPool, framesInFlight, swapchain.GetWidth (), swapchain.GetHeight ())
    {
    }

    GraphSettings (Device& device, VkQueue queue, VkCommandPool commandPool, const Swapchain& swapchain)
        : GraphSettings (device, queue, commandPool, swapchain, swapchain.GetImageCount ())
    {
    }
};

#endif