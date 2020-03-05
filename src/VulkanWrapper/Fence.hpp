#ifndef FENCE_HPP
#define FENCE_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

class Fence : public Noncopyable {
private:
    const VkDevice device;
    const VkFence  handle;

    static VkFence CreateFence (VkDevice device)
    {
        VkFence           handle;
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;
        if (ERROR (vkCreateFence (device, &fenceInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create fence");
        }
        return handle;
    }

public:
    USING_PTR (Fence);

    Fence (VkDevice device)
        : device (device)
        , handle (CreateFence (device))
    {
    }

    ~Fence ()
    {
        vkDestroyFence (device, handle, nullptr);
    }

    operator VkFence () const
    {
        return handle;
    }
};
#endif