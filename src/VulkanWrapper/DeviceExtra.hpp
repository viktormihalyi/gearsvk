#ifndef DEVICEEXTRA_HPP
#define DEVICEEXTRA_HPP

#include "CommandPool.hpp"
#include "Device.hpp"
#include "Queue.hpp"

struct DeviceExtra {
    Device&      device;
    CommandPool& commandPool;
    Queue&       graphicsQueue;
};

#endif