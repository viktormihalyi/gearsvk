#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "MovablePtr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"
#include "vk_mem_alloc.h"

namespace GVK {

class GVK_RENDERER_API Buffer : public VulkanObject {
private:
    VmaAllocator                   allocator;
    GVK::MovablePtr<VkBuffer>      handle;
    GVK::MovablePtr<VmaAllocation> allocationHandle;
    size_t                         size;

public:
    enum class MemoryLocation {
        GPU,
        CPU
    };

    Buffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc);
    Buffer (Buffer&&) = default;
    Buffer& operator= (Buffer&&) = default;

    virtual ~Buffer () override;

    operator VkBuffer () const { return handle; }

    operator VmaAllocation () const { return allocationHandle; }
};


class GVK_RENDERER_API UniformBuffer : public Buffer {
public:
    UniformBuffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
        : Buffer (allocator, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags, loc)
    {
    }
};


class GVK_RENDERER_API StorageBuffer : public Buffer {
public:
    StorageBuffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
        : Buffer (allocator, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | usageFlags, loc)
    {
    }
};


class GVK_RENDERER_API IndexBuffer : public Buffer {
public:
    IndexBuffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
        : Buffer (allocator, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | usageFlags, loc)
    {
    }
};


class GVK_RENDERER_API VertexBuffer : public Buffer {
public:
    VertexBuffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
        : Buffer (allocator, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usageFlags, loc)
    {
    }
};

} // namespace GVK

#endif