#ifndef DEVICEEXTRA_HPP
#define DEVICEEXTRA_HPP

#include "Ptr.hpp"

#include "CommandPool.hpp"
#include "Device.hpp"
#include "Queue.hpp"

class DeviceExtra : public DeviceInterface {
public:
    Device&      device;
    CommandPool& commandPool;
    Queue&       graphicsQueue;
    Queue&       presentationQueue = dummyQueue;

    USING_PTR (DeviceExtra);

    DeviceExtra (Device& device, CommandPool& commandPool, Queue& graphicsQueue, Queue& presentationQueue = dummyQueue)
        : device (device)
        , commandPool (commandPool)
        , graphicsQueue (graphicsQueue)
        , presentationQueue (presentationQueue)
    {
    }

    Device&      GetDevice () { return device; }
    CommandPool& GetCommandPool () { return commandPool; }
    Queue&       GetGraphicsQueue () { return graphicsQueue; }
    Queue&       GetPresentationQueue () { return presentationQueue; }

    operator Device& () { return device; }

    // implementing DeviceInterface
    virtual              operator VkDevice () const { return device; }
    virtual void         Wait () const { device.Wait (); }
    virtual AllocateInfo GetImageAllocateInfo (VkImage image, VkMemoryPropertyFlags propertyFlags) const { return device.GetImageAllocateInfo (image, propertyFlags); }
    virtual AllocateInfo GetBufferAllocateInfo (VkBuffer buffer, VkMemoryPropertyFlags propertyFlags) const { return device.GetBufferAllocateInfo (buffer, propertyFlags); }
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