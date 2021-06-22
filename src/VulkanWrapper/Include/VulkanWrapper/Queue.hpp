#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <vulkan/vulkan.h>

#include "VulkanWrapper/VulkanWrapperAPI.hpp"

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "Utils/Utils.hpp"

namespace GVK {

class CommandBuffer;

class VULKANWRAPPER_API Queue : public Noncopyable {
private:
    GVK::MovablePtr<VkQueue> handle;

public:
    Queue (VkDevice device, uint32_t index)
    {
        vkGetDeviceQueue (device, index, 0, &handle); // only one queue per device
    }

    Queue (VkQueue handle)
        : handle (handle)
    {
    }

    ~Queue ()
    {
        handle = nullptr;
    }

    operator VkQueue () const
    {
        return handle;
    }

    void Wait () const
    {
        vkQueueWaitIdle (handle);
    }

    void Submit (const std::vector<VkSemaphore>&          waitSemaphores,
                 const std::vector<VkPipelineStageFlags>& waitDstStageMasks,
                 const std::vector<CommandBuffer*>&       commandBuffers,
                 const std::vector<VkSemaphore>&          signalSemaphores,
                 VkFence                                  fenceToSignal) const;
};

VULKANWRAPPER_API extern Queue dummyQueue;

} // namespace GVK

#endif