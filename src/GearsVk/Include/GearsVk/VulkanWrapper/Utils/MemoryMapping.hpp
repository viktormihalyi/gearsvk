#ifndef MEMORYMAPPING_HPP
#define MEMORYMAPPING_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "MovablePtr.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

#include "vk_mem_alloc.h"

#include <cstring>

namespace GVK {

class GVK_RENDERER_API MemoryMapping {
private:
    GVK::MovablePtr<VkDevice>       device;
    GVK::MovablePtr<VkDeviceMemory> memory;

    GVK::MovablePtr<VmaAllocator>  allocator;
    GVK::MovablePtr<VmaAllocation> allocationHandle;

    size_t offset;
    size_t size;

    GVK::MovablePtr<void*> mappedMemory;

public:
    MemoryMapping (VkDevice device, VkDeviceMemory memory, size_t offset, size_t size)
        : device (device)
        , allocator (VK_NULL_HANDLE)
        , allocationHandle (VK_NULL_HANDLE)
        , memory (memory)
        , offset (offset)
        , size (size)
        , mappedMemory (nullptr)
    {
        if (GVK_ERROR (vkMapMemory (device, memory, offset, size, 0, &mappedMemory) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to map memory");
        }
    }

    MemoryMapping (VkDevice device, const DeviceMemory& memory)
        : MemoryMapping (device, memory, 0, memory.GetSize ())
    {
    }

    MemoryMapping (VmaAllocator allocator, VmaAllocation allocationHandle)
        : device (VK_NULL_HANDLE)
        , memory (VK_NULL_HANDLE)
        , allocator (allocator)
        , allocationHandle (allocationHandle)
        , offset (0)
        , size (0)
        , mappedMemory (nullptr)
    {
        VmaAllocationInfo allocInfo = {};
        vmaGetAllocationInfo (allocator, allocationHandle, &allocInfo);
        size = allocInfo.size;

        if (GVK_ERROR (vmaMapMemory (allocator, allocationHandle, &mappedMemory) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to map memory");
        }
    }

    ~MemoryMapping ()
    {
        if (device != VK_NULL_HANDLE) {
            vkUnmapMemory (device, memory);
        } else if (allocator != VK_NULL_HANDLE) {
            vmaUnmapMemory (allocator, allocationHandle);
        }
        mappedMemory = nullptr;
    }

    template<typename T>
    void Copy (const std::vector<T>& obj) const
    {
        const size_t copiedObjSize = sizeof (T);
        GVK_ASSERT (copiedObjSize * obj.size () == size);
        memcpy (mappedMemory, obj.data (), size);
    }

    template<typename T>
    void Copy (const T& obj) const
    {
        const size_t copiedObjSize = sizeof (T);
        GVK_ASSERT (copiedObjSize <= size);
        memcpy (mappedMemory, &obj, size);
    }

    void Copy (const void* data, size_t copiedSize) const
    {
        if (GVK_ERROR (copiedSize > size)) {
            throw std::runtime_error ("overflow");
        }

        memcpy (mappedMemory, reinterpret_cast<const uint8_t*> (data), copiedSize);
    }

    void*    Get () const { return mappedMemory; }
    uint32_t GetSize () { return size; }
    uint32_t GetOffset () { return offset; }
};

} // namespace GVK

#endif