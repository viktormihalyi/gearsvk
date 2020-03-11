#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

class Semaphore : public Noncopyable {
private:
    const VkDevice    device;
    const VkSemaphore handle;

    static VkSemaphore CreateSemaphore (VkDevice device, bool signaledByDefault)
    {
        VkSemaphore           handle;
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.flags                 = (signaledByDefault) ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
        if (ERROR (vkCreateSemaphore (device, &semaphoreInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create semaphore");
        }
        return handle;
    }

public:
    USING_PTR (Semaphore);

    Semaphore (VkDevice device, bool signaledByDefault = false)
        : device (device)
        , handle (CreateSemaphore (device, signaledByDefault))
    {
    }

    ~Semaphore ()
    {
        vkDestroySemaphore (device, handle, nullptr);
    }

    operator VkSemaphore () const
    {
        return handle;
    }
};

#endif