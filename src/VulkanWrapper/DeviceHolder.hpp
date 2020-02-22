#ifndef VW_DEVICEHOLDER_HPP
#define VW_DEVICEHOLDER_HPP

#include <vulkan/vulkan.h>

class DeviceHolder {
protected:
    VkDevice device;

protected:
    DeviceHolder (VkDevice device)
        : device (device)
    {
    }

    virtual ~DeviceHolder ()
    {
    }
};

#endif