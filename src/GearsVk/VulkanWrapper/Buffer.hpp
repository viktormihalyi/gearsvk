#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"
#include "vk_mem_alloc.h"


USING_PTR (Buffer);
class GEARSVK_API Buffer : public VulkanObject {
    USING_CREATE (Buffer);

private:
    const VmaAllocator allocator;
    VkBuffer           handle;
    VmaAllocation      allocationHandle;
    const size_t       size;

public:
    enum class MemoryLocation {
        GPU,
        CPU
    };

    Buffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc);

    virtual ~Buffer () override;

    operator VkBuffer () const { return handle; }

    operator VmaAllocation () const { return allocationHandle; }
};


USING_PTR (UniformBuffer);
class UniformBuffer : public Buffer {
    USING_CREATE (UniformBuffer);

public:
    UniformBuffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
        : Buffer (allocator, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags, loc)
    {
    }
};


USING_PTR (StorageBuffer);
class StorageBuffer : public Buffer {
    USING_CREATE (StorageBuffer);

public:
    StorageBuffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
        : Buffer (allocator, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | usageFlags, loc)
    {
    }
};


USING_PTR (IndexBuffer);
class IndexBuffer : public Buffer {
    USING_CREATE (IndexBuffer);

public:
    IndexBuffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
        : Buffer (allocator, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | usageFlags, loc)
    {
    }
};


USING_PTR (VertexBuffer);
class VertexBuffer : public Buffer {
    USING_CREATE (VertexBuffer);

public:
    VertexBuffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
        : Buffer (allocator, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usageFlags, loc)
    {
    }
};

#endif