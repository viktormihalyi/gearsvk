#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

USING_PTR (Semaphore);
class GVK_RENDERER_API Semaphore : public VulkanObject {
private:
    const VkDevice device;
    VkSemaphore    handle;

    static VkSemaphore CreateSemaphore (VkDevice device)
    {
        VkSemaphore           handle;
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.flags                 = 0;
        if (GVK_ERROR (vkCreateSemaphore (device, &semaphoreInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create semaphore");
        }
        return handle;
    }

public:
    USING_CREATE (Semaphore);

    Semaphore (VkDevice device)
        : device (device)
        , handle (CreateSemaphore (device))
    {
    }

    ~Semaphore ()
    {
        vkDestroySemaphore (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkSemaphore () const
    {
        return handle;
    }
};

#endif