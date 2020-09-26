#ifndef SINGLETIMECOMMAND_HPP
#define SINGLETIMECOMMAND_HPP

#include "Assert.hpp"
#include "CommandBuffer.hpp"
#include "DeviceExtra.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>

USING_PTR (SingleTimeCommand);

class GEARSVK_API SingleTimeCommand final : public Noncopyable {
private:
    const VkDevice      device;
    const VkCommandPool commandPool;
    const Queue&        queue;
    CommandBuffer       commandBuffer;

public:
    SingleTimeCommand (VkDevice device, VkCommandPool commandPool, const Queue& queue)
        : device (device)
        , queue (queue)
        , commandPool (commandPool)
        , commandBuffer (device, commandPool)
    {
        commandBuffer.Begin (VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    }

    SingleTimeCommand (const DeviceExtra& device)
        : SingleTimeCommand (device, device.GetCommandPool (), device.GetGraphicsQueue ())
    {
    }

    ~SingleTimeCommand ()
    {
        commandBuffer.End ();

        queue.Submit ({}, {}, { &commandBuffer }, {}, VK_NULL_HANDLE);
        queue.Wait ();
    }

    CommandBuffer& Record () { return commandBuffer; }
};

#endif