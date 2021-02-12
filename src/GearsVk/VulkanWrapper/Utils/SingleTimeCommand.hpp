#ifndef SINGLETIMECOMMAND_HPP
#define SINGLETIMECOMMAND_HPP

#include "Assert.hpp"
#include "CommandBuffer.hpp"
#include "DeviceExtra.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>

namespace GVK {

USING_PTR (SingleTimeCommand);
class GVK_RENDERER_API SingleTimeCommand final : public CommandBuffer {
private:
    const Queue&        queue;

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

}

#endif