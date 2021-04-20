#include "Buffer.hpp"

namespace GVK {

template<typename T>
class VulkanObjectHandleHolder final {
private:
    T handle;

public:
    VulkanObjectHandleHolder ()
        : handle (VK_NULL_HANDLE)
    {
    }

    VulkanObjectHandleHolder (T handle)
        : handle (handle)
    {
    }

    ~VulkanObjectHandleHolder ()
    {
        handle = nullptr;
    }

    VulkanObjectHandleHolder (const VulkanObjectHandleHolder&) = delete;
    VulkanObjectHandleHolder& operator= (VulkanObjectHandleHolder&) = delete;

    VulkanObjectHandleHolder (VulkanObjectHandleHolder&& other)
    {
        handle       = other.handle;
        other.handle = nullptr;
    }

    VulkanObjectHandleHolder& operator= (VulkanObjectHandleHolder&& other)
    {
        handle = nullptr;
        std::swap (handle, other.handle);
        return *this;
    }

    operator T () { return handle; }
};

Buffer::Buffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
    : allocator (allocator)
    , handle (VK_NULL_HANDLE)
    , allocationHandle (VK_NULL_HANDLE)
    , size (size)
{
    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size               = bufferSize;
    bufferInfo.usage              = usageFlags;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage                   = (loc == MemoryLocation::GPU) ? VMA_MEMORY_USAGE_GPU_ONLY : VMA_MEMORY_USAGE_CPU_COPY;

    if (loc == MemoryLocation::CPU) {
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }

    if (GVK_ERROR (vmaCreateBuffer (allocator, &bufferInfo, &allocInfo, &handle, &allocationHandle, nullptr) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create vma buffer");
    }
}


Buffer::~Buffer ()
{
    if (allocator != VK_NULL_HANDLE) {
        vmaDestroyBuffer (allocator, handle, allocationHandle);
    }

    handle = nullptr;
}

} // namespace GVK