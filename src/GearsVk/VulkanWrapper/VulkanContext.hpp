#ifndef VULKANCONTEXT_HPP
#define VULKANCONTEXT_HPP

#include "CommandPool.hpp"
#include "Device.hpp"
#include "Queue.hpp"


class VulkanContext final {
public:
    DeviceObject& device;
    CommandPool&  commandPool;
    Queue&        graphicsQueue;

    VulkanContext (DeviceObject& device,
                   CommandPool&  commandPool,
                   Queue&        graphicsQueue)
        : device (device)
        , commandPool (commandPool)
        , graphicsQueue (graphicsQueue)
    {
    }
};

#endif
