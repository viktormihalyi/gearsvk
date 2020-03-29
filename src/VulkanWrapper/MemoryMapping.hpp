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

    const uint32_t offset;
    const uint32_t size;

    void* mappedMemory;

public:
    USING_PTR (MemoryMapping);

    MemoryMapping (VkDevice device, VkDeviceMemory memory, uint32_t offset, uint32_t size)
        : device (device)
        , memory (memory)
        , offset (offset)
        , size (size)
        , mappedMemory (nullptr)
    {
        if (ERROR (vkMapMemory (device, memory, offset, size, 0, &mappedMemory) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to map memory");
        }
    }

    MemoryMapping (VkDevice device, const DeviceMemory& memory)
        : MemoryMapping (device, memory, 0, memory.GetSize ())
    {
    }

    ~MemoryMapping ()
    {
        vkUnmapMemory (device, memory);
        mappedMemory = nullptr;
    }

    template<typename T>
    void Copy (const std::vector<T>& obj) const
    {
        const uint32_t copiedObjSize = sizeof (T);
        ASSERT (copiedObjSize * obj.size () == size);
        memcpy (mappedMemory, obj.data (), size);
    }

    template<typename T>
    void Copy (const T& obj) const
    {
        const uint32_t copiedObjSize = sizeof (T);
        ASSERT (copiedObjSize == size);
        memcpy (mappedMemory, &obj, size);
    }

    void Copy (const void* data, uint32_t size, uint32_t offset) const
    {
        if (ERROR (size + offset > size)) {
            throw std::runtime_error ("overflow");
        }

        memcpy (mappedMemory, reinterpret_cast<const char*> (data) + offset, size);
    }

    void* Get () const { return mappedMemory; }
};

#endif