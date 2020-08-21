#ifndef DEVICEEXTRA_HPP
#define DEVICEEXTRA_HPP

#include "Ptr.hpp"

#include "CommandPool.hpp"
#include "Device.hpp"
#include "Queue.hpp"

USING_PTR (DeviceExtra);

class GEARSVK_API DeviceExtra : public Device {
public:
    Device&      device;
    CommandPool& commandPool;
    Queue&       graphicsQueue;
    Queue&       presentationQueue = dummyQueue;

    USING_CREATE (DeviceExtra);

    DeviceExtra (Device& device, CommandPool& commandPool, Queue& graphicsQueue, Queue& presentationQueue = dummyQueue)
        : device (device)
        , commandPool (commandPool)
        , graphicsQueue (graphicsQueue)
        , presentationQueue (presentationQueue)
    {
    }

    const Device&      GetDevice () const { return device; }
    const CommandPool& GetCommandPool () const { return commandPool; }
    const Queue&       GetGraphicsQueue () const { return graphicsQueue; }
    const Queue&       GetPresentationQueue () const { return presentationQueue; }

    Device&      GetDevice () { return device; }
    CommandPool& GetCommandPool () { return commandPool; }
    Queue&       GetGraphicsQueue () { return graphicsQueue; }
    Queue&       GetPresentationQueue () { return presentationQueue; }

    // implementing Device
    virtual              operator VkDevice () const override { return device; }
    virtual void         Wait () const override { device.Wait (); }
    virtual AllocateInfo GetImageAllocateInfo (VkImage image, VkMemoryPropertyFlags propertyFlags) const override { return device.GetImageAllocateInfo (image, propertyFlags); }
    virtual AllocateInfo GetBufferAllocateInfo (VkBuffer buffer, VkMemoryPropertyFlags propertyFlags) const override { return device.GetBufferAllocateInfo (buffer, propertyFlags); }
};


class DeviceExtraHolder {
public:
    DeviceExtra& device;

    DeviceExtraHolder (DeviceExtra& device)
        : device (device)
    {
    }
};

#endif