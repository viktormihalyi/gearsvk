#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"


class Buffer : public Noncopyable {
private:
    const VkDevice device;
    const VkBuffer handle;

    static VkBuffer CreateBuffer (VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags)
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size               = bufferSize;
        bufferInfo.usage              = usageFlags;
        bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

        VkBuffer result;
        if (ERROR (vkCreateBuffer (device, &bufferInfo, nullptr, &result) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create buffer");
        }
        return result;
    }

public:
    USING_PTR (Buffer);

    Buffer (VkDevice device, size_t bufferSize, VkBufferUsageFlags usageFlags)
        : device (device)
        , handle (CreateBuffer (device, bufferSize, usageFlags))
    {
    }

    ~Buffer ()
    {
        vkDestroyBuffer (device, handle, nullptr);
    }

    operator VkBuffer () const
    {
        return handle;
    }
};

#endif