#ifndef DEVICEEXTRA_HPP
#define DEVICEEXTRA_HPP

#include "Ptr.hpp"

#include "CommandPool.hpp"
#include "Device.hpp"
#include "Queue.hpp"

#include "vk_mem_alloc.h"

namespace GVK {

class GVK_RENDERER_API DeviceExtra : public Device {
public:
    Device&      device;
    CommandPool& commandPool;
    Queue&       graphicsQueue;
    Queue&       presentationQueue;
    VmaAllocator allocator;


    DeviceExtra (Device& device, CommandPool& commandPool, VmaAllocator allocator, Queue& graphicsQueue, Queue& presentationQueue = dummyQueue)
        : device (device)
        , commandPool (commandPool)
        , graphicsQueue (graphicsQueue)
        , presentationQueue (presentationQueue)
        , allocator (allocator)
    {
    }

    const Device&      GetDevice () const { return device; }
    const CommandPool& GetCommandPool () const { return commandPool; }
    const Queue&       GetGraphicsQueue () const { return graphicsQueue; }
    const Queue&       GetPresentationQueue () const { return presentationQueue; }
    VmaAllocator       GetAllocator () const { return allocator; }

    Device&      GetDevice () { return device; }
    CommandPool& GetCommandPool () { return commandPool; }
    Queue&       GetGraphicsQueue () { return graphicsQueue; }
    Queue&       GetPresentationQueue () { return presentationQueue; }

    // implementing Device
    virtual      operator VkDevice () const override { return device; }
    virtual void Wait () const override { device.Wait (); }
};

} // namespace GVK

#endif