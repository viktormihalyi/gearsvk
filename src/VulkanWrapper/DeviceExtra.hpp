#ifndef DEVICEEXTRA_HPP
#define DEVICEEXTRA_HPP

#include "CommandPool.hpp"
#include "Device.hpp"
#include "Queue.hpp"

struct DeviceExtra {
    Device&      device;
    CommandPool& commandPool;
    Queue&       graphicsQueue;
    Queue&       presentationQueue = dummyQueue;

    Device&      GetDevice () { return device; }
    CommandPool& GetCommandPool () { return commandPool; }
    Queue&       GetGraphicsQueue () { return graphicsQueue; }
    Queue&       GetPresentationQueue () { return presentationQueue; }
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