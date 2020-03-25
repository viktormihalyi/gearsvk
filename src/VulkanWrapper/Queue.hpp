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
    USING_PTR (Queue);

    Queue (VkDevice device, uint32_t index)
    {
        vkGetDeviceQueue (device, index, 0, &handle); // TODO another index
    }

    ~Queue ()
    {
        handle = VK_NULL_HANDLE;
    }

    operator VkQueue () const
    {
        return handle;
    }
};

#endif