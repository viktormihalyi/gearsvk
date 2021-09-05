#ifndef MEMORYMAPPING_HPP
#define MEMORYMAPPING_HPP

#include <vulkan/vulkan.h>

#include "VulkanWrapper/VulkanWrapperAPI.hpp"

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "Utils/Noncopyable.hpp"

#pragma warning (push, 0)
#include "vk_mem_alloc.h"
#pragma warning(pop)

#include <cstring>
#include <vector>

namespace GVK {

class DeviceMemory;

class VULKANWRAPPER_API MemoryMapping {
private:
    GVK::MovablePtr<VkDevice>       device;
    GVK::MovablePtr<VkDeviceMemory> memory;

    GVK::MovablePtr<VmaAllocator>  allocator;
    GVK::MovablePtr<VmaAllocation> allocationHandle;

    size_t offset;
    size_t size;

    GVK::MovablePtr<void*> mappedMemory;

public:
    MemoryMapping (VkDevice device, VkDeviceMemory memory, size_t offset, size_t size);
    MemoryMapping (VkDevice device, const DeviceMemory& memory);
    MemoryMapping (VmaAllocator allocator, VmaAllocation allocationHandle);

    ~MemoryMapping ();

    template<typename T>
    void Copy (const std::vector<T>& obj) const
    {
        GVK_ASSERT (sizeof (T) * obj.size () == size);
        memcpy (mappedMemory, obj.data (), sizeof (T) * obj.size ());
    }

    template<typename T>
    void Copy (const T& obj) const
    {
        const size_t copiedObjSize = sizeof (T);
        GVK_ASSERT (copiedObjSize <= size);
        memcpy (mappedMemory, &obj, size);
    }

    void Copy (const void* data, size_t copiedSize) const;

    void*    Get () const { return mappedMemory; }
    uint32_t GetSize () { return size; }
    uint32_t GetOffset () { return offset; }
};

} // namespace GVK

#endif