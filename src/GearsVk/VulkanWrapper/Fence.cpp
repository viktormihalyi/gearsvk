#include "Fence.hpp"

#include "Assert.hpp"
#include "Time.hpp"

#include <iostream>


constexpr bool LOG_WAITS = true;


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


void Fence::WaitImpl () const
{
    vkWaitForFences (device, 1, &handle, VK_TRUE, UINT64_MAX);
}


void Fence::Wait () const
{
    if constexpr (LOG_WAITS) {
        
        const GVK::TimePoint start = GVK::TimePoint::SinceEpoch ();

        WaitImpl ();
        
        const GVK::TimePoint end = GVK::TimePoint::SinceEpoch ();

        const std::string fenceId = !GetName ().empty () ? GetName () : GetUUID ().GetValue ();
        std::cout << "fence \"" << fenceId << "\" waited " << std::fixed << (end - start).AsMilliseconds () << " ms" << std::endl;

    } else {
        
        WaitImpl ();

    }
}


void Fence::Reset () const
{
    vkResetFences (device, 1, &handle);
}

} // namespace GVK
