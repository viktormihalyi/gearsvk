#ifndef GRAPHSETTINGS_HPP
#define GRAPHSETTINGS_HPP

#include <vulkan/vulkan.h>
#include "DeviceExtra.hpp"

namespace RG {

struct GraphSettings {
    const DeviceExtra* device;
    uint32_t           framesInFlight;

    GraphSettings (const DeviceExtra& device, VkQueue queue, VkCommandPool commandPool, uint32_t framesInFlight)
        : device (&device)
        , framesInFlight (framesInFlight)
    {
    }

    GraphSettings (const DeviceExtra& device, uint32_t framesInFlight)
        : GraphSettings (device, device.GetGraphicsQueue (), device.GetCommandPool (), framesInFlight)
    {
    }


    GraphSettings ()
        : device (nullptr)
        , framesInFlight (0)
    {
    }

    const DeviceExtra& GetDevice () const { return *device; }
    const Queue&       GetGrahpicsQueue () const { return device->GetGraphicsQueue (); }
    const CommandPool& GetCommandPool () const { return device->GetCommandPool (); }
};

} // namespace RG

#endif