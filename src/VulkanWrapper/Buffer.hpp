#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"


class Buffer : public Noncopyable {
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

    ~Buffer ()
    {
        vkDestroyBuffer (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkBuffer () const
    {
        return handle;
    }
};

#endif