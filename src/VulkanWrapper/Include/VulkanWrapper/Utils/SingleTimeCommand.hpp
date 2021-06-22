#ifndef SINGLETIMECOMMAND_HPP
#define SINGLETIMECOMMAND_HPP

#include "Utils/Assert.hpp"
#include "VulkanWrapper/CommandBuffer.hpp"
#include "VulkanWrapper/DeviceExtra.hpp"
#include "Utils/Noncopyable.hpp"
#include "Utils/Utils.hpp"

#include <vulkan/vulkan.h>

namespace GVK {

class /* VULKANWRAPPER_API */ SingleTimeCommand final : public CommandBuffer {
private:
    const Queue& queue;

public:
    SingleTimeCommand (VkDevice device, VkCommandPool commandPool, const Queue& queue)
        : CommandBuffer (device, commandPool)
        , queue (queue)
    {
        Begin (VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    }

    SingleTimeCommand (const DeviceExtra& device)
        : SingleTimeCommand (device, device.GetCommandPool (), device.GetGraphicsQueue ())
    {
    }

    virtual ~SingleTimeCommand () override
    {
        End ();

        queue.Submit ({}, {}, { this }, {}, VK_NULL_HANDLE);
        queue.Wait ();
    }
};

} // namespace GVK

#endif