#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

USING_PTR_2 (Buffer);

class GEARSVK_API Buffer : public Noncopyable {
private:
    const VkDevice device;
    VkBuffer       handle;

public:
    USING_PTR (Buffer);

    Buffer (VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags)
        : device (device)
        , handle (VK_NULL_HANDLE)
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size               = bufferSize;
        bufferInfo.usage              = usageFlags;
        bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

        if (ERROR (vkCreateBuffer (device, &bufferInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create buffer");
        }
    }

    virtual ~Buffer ()
    {
        vkDestroyBuffer (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkBuffer () const
    {
        return handle;
    }
};

USING_PTR_2 (UniformBuffer);

class UniformBuffer : public Buffer {
public:
    USING_PTR (UniformBuffer);
    UniformBuffer (VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags = 0)
        : Buffer (device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags)
    {
    }
};

USING_PTR_2 (StorageBuffer);

class StorageBuffer : public Buffer {
public:
    USING_PTR (StorageBuffer);
    StorageBuffer (VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags = 0)
        : Buffer (device, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | usageFlags)
    {
    }
};

USING_PTR_2 (IndexBuffer);

class IndexBuffer : public Buffer {
public:
    USING_PTR (IndexBuffer);
    IndexBuffer (VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags = 0)
        : Buffer (device, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | usageFlags)
    {
    }
};

USING_PTR_2 (VertexBuffer);

class VertexBuffer : public Buffer {
public:
    USING_PTR (VertexBuffer);
    VertexBuffer (VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags = 0)
        : Buffer (device, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usageFlags)
    {
    }
};

#endif