#ifndef MEMORYMAPPING_HPP
#define MEMORYMAPPING_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

#include <cstring>

USING_PTR (MemoryMapping);
class GEARSVK_API MemoryMapping : public Noncopyable {
private:
    const VkDevice       device;
    const VkDeviceMemory memory;

    const size_t offset;
    const size_t size;

    void* mappedMemory;

public:
    USING_CREATE (MemoryMapping);

    MemoryMapping (VkDevice device, VkDeviceMemory memory, size_t offset, size_t size)
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
        const size_t copiedObjSize = sizeof (T);
        ASSERT (copiedObjSize * obj.size () == size);
        memcpy (mappedMemory, obj.data (), size);
    }

    template<typename T>
    void Copy (const T& obj) const
    {
        const size_t copiedObjSize = sizeof (T);
        ASSERT (copiedObjSize == size);
        memcpy (mappedMemory, &obj, size);
    }

    void Copy (const void* data, size_t copiedSize, size_t copiedOffset) const
    {
        if (ERROR (copiedSize + copiedOffset > size)) {
            throw std::runtime_error ("overflow");
        }

        memcpy (mappedMemory, reinterpret_cast<const uint8_t*> (data) + copiedOffset, copiedSize);
    }

    void*    Get () const { return mappedMemory; }
    uint32_t GetSize () { return size; }
    uint32_t GetOffset () { return offset; }
};

#endif