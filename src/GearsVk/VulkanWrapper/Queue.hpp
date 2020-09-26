#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <vulkan/vulkan.h>

#include "GearsVkAPI.hpp"

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

class CommandBuffer;

USING_PTR (Queue);
class GEARSVK_API Queue : public Noncopyable {
private:
    VkQueue handle;

public:
    USING_CREATE (Queue);

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
        handle = VK_NULL_HANDLE;
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

extern Queue dummyQueue;

#endif