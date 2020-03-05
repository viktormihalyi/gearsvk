#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

class Queue : public Noncopyable {
private:
    VkQueue handle;

public:
    Queue (VkDevice device, uint32_t index)
    {
        vkGetDeviceQueue (device, index, 0, &handle); // TODO another index
    }

    operator VkQueue () const
    {
        return handle;
    }
};

#endif