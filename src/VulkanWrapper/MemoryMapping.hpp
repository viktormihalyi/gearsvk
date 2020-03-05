#ifndef MEMORYMAPPING_HPP
#define MEMORYMAPPING_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"


class MemoryMapping : public Noncopyable {
private:
    const VkDevice       device;
    const VkDeviceMemory memory;

    void* mappedMemory;

public:
    USING_PTR (MemoryMapping);

    MemoryMapping (VkDevice device, VkDeviceMemory memory, uint32_t offset, uint32_t size)
        : device (device)
        , memory (memory)
        , mappedMemory (nullptr)

    {
        if (ERROR (vkMapMemory (device, memory, offset, size, 0, &mappedMemory) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to map memory");
        }
    }

    ~MemoryMapping ()
    {
        vkUnmapMemory (device, memory);
    }

    void* Get () const
    {
        return mappedMemory;
    }
};

#endif