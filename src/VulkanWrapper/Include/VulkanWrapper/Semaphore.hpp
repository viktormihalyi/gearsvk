#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include <vulkan/vulkan.h>

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "Utils/Utils.hpp"
#include "VulkanObject.hpp"

namespace GVK {

class /* VULKANWRAPPER_API */ Semaphore : public VulkanObject {
private:
    VkDevice                     device;
    GVK::MovablePtr<VkSemaphore> handle;

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
    Semaphore (VkDevice device)
        : device (device)
        , handle (CreateSemaphore (device))
    {
    }

    Semaphore (Semaphore&&) = default;
    Semaphore& operator= (Semaphore&&) = default;

    ~Semaphore ()
    {
        vkDestroySemaphore (device, handle, nullptr);
        handle = nullptr;
    }

    operator VkSemaphore () const
    {
        return handle;
    }
};

} // namespace GVK

#endif