#include "Fence.hpp"

#include "Assert.hpp"
#include "Time.hpp"

#include <iostream>


#define LOG_WAITS 0


namespace GVK {

Fence::Fence (VkDevice device)
    : device (device)
    , handle (VK_NULL_HANDLE)
{
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;
    if (GVK_ERROR (vkCreateFence (device, &fenceInfo, nullptr, &handle) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create fence");
    }
}


Fence::~Fence ()
{
    vkDestroyFence (device, handle, nullptr);
    handle = nullptr;
}


void Fence::Wait () const
{
#if LOG_WAITS
    const GVK::TimePoint start = GVK::TimePoint::SinceEpoch ();
#endif

    vkWaitForFences (device, 1, &handle, VK_TRUE, UINT64_MAX);

#if LOG_WAITS
    const GVK::TimePoint end = GVK::TimePoint::SinceEpoch ();
    std::cout << "fence " << GetUUID ().GetValue () << " waited " << std::fixed << (end - start).AsMilliseconds () << " ms" << std::endl;
#endif
}


void Fence::Reset () const
{
    vkResetFences (device, 1, &handle);
}

} // namespace GVK
