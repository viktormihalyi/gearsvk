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
    VkFence        handle;

public:
    USING_PTR (Fence);

    Fence (VkDevice device)
        : device (device)
        , handle (VK_NULL_HANDLE)
    {
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;
        if (ERROR (vkCreateFence (device, &fenceInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create fence");
        }
    }

    ~Fence ()
    {
        vkDestroyFence (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkFence () const
    {
        return handle;
    }

    void Wait () const
    {
        vkWaitForFences (device, 1, &handle, VK_TRUE, UINT64_MAX);
    }

    void Reset () const
    {
        vkResetFences (device, 1, &handle);
    }
};
#endif