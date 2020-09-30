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
private:
    const VkDevice     device;
    const VmaAllocator allocator;
    VkBuffer           handle;
    VmaAllocation      allocationHandle;

public:
    USING_CREATE (Buffer);

    Buffer (VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags)
        : device (device)
        , allocator (VK_NULL_HANDLE)
        , handle (VK_NULL_HANDLE)
        , allocationHandle (VK_NULL_HANDLE)
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size               = bufferSize;
        bufferInfo.usage              = usageFlags;
        bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

        if (GVK_ERROR (vkCreateBuffer (device, &bufferInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create buffer");
        }
    }

    enum MemoryLocation {
        GPU,
        CPU
    };

    Buffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
        : device (VK_NULL_HANDLE)
        , allocator (allocator)
        , handle (VK_NULL_HANDLE)
        , allocationHandle (VK_NULL_HANDLE)
    {
        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size               = bufferSize;
        bufferInfo.usage              = usageFlags;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage                   = (loc == MemoryLocation::GPU) ? VMA_MEMORY_USAGE_GPU_ONLY : VMA_MEMORY_USAGE_CPU_COPY;

        if (GVK_ERROR (vmaCreateBuffer (allocator, &bufferInfo, &allocInfo, &handle, &allocationHandle, nullptr) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create vma buffer");
        }
    }

    virtual ~Buffer ()
    {
        if (device != VK_NULL_HANDLE) {
            vkDestroyBuffer (device, handle, nullptr);
        } else if (allocator != VK_NULL_HANDLE) {
            vmaDestroyBuffer (allocator, handle, allocationHandle);
        }

        handle = VK_NULL_HANDLE;
    }

    operator VkBuffer () const { return handle; }

    operator VmaAllocation () const { return allocationHandle; }
};

USING_PTR (UniformBuffer);

class UniformBuffer : public Buffer {
public:
    USING_CREATE (UniformBuffer);
    UniformBuffer (VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags = 0)
        : Buffer (device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags)
    {
    }
    UniformBuffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
        : Buffer (allocator, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags, loc)
    {
    }
};

USING_PTR (StorageBuffer);

class StorageBuffer : public Buffer {
public:
    USING_CREATE (StorageBuffer);
    StorageBuffer (VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags = 0)
        : Buffer (device, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | usageFlags)
    {
    }
    StorageBuffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
        : Buffer (allocator, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | usageFlags, loc)
    {
    }
};

USING_PTR (IndexBuffer);

class IndexBuffer : public Buffer {
public:
    USING_CREATE (IndexBuffer);
    IndexBuffer (VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags = 0)
        : Buffer (device, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | usageFlags)
    {
    }
    IndexBuffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
        : Buffer (allocator, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | usageFlags, loc)
    {
    }
};

USING_PTR (VertexBuffer);

class VertexBuffer : public Buffer {
public:
    USING_CREATE (VertexBuffer);
    VertexBuffer (VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags = 0)
        : Buffer (device, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usageFlags)
    {
    }
    VertexBuffer (VmaAllocator allocator, size_t bufferSize, VkBufferUsageFlags usageFlags, MemoryLocation loc)
        : Buffer (allocator, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usageFlags, loc)
    {
    }
};

#endif