#include "Queue.hpp"

#include "CommandBuffer.hpp"
#include <iostream>

namespace GVK {

Queue dummyQueue (VK_NULL_HANDLE);


void Queue::Submit (const std::vector<VkSemaphore>&          waitSemaphores,
                    const std::vector<VkPipelineStageFlags>& waitDstStageMasks,
                    const std::vector<CommandBuffer*>&       commandBuffers,
                    const std::vector<VkSemaphore>&          signalSemaphores,
                    VkFence                                  fenceToSignal) const
{
#if 0
    std::cout << "submitting:" << std::endl;
    for (CommandBuffer* cmd : commandBuffers) {
        std::cout << "    " << cmd->GetUUID ().GetValue () << std::endl;
        for (CommandBuffer::CommandType cmdType : cmd->recordedCommands) {
            std::cout << "        " << CommandBuffer::CommandTypeToString (cmdType) << std::endl;
        }
    }
#endif

    std::vector<VkCommandBuffer> submittedCmdBufferHandles;
    for (CommandBuffer* cmd : commandBuffers) {
        GVK_ASSERT (cmd != nullptr);
        submittedCmdBufferHandles.push_back (cmd->GetHandle ());
    }

    VkSubmitInfo result         = {};
    result.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    result.waitSemaphoreCount   = static_cast<uint32_t> (waitSemaphores.size ());
    result.pWaitSemaphores      = waitSemaphores.data ();
    result.pWaitDstStageMask    = waitDstStageMasks.data ();
    result.commandBufferCount   = static_cast<uint32_t> (submittedCmdBufferHandles.size ());
    result.pCommandBuffers      = submittedCmdBufferHandles.data ();
    result.signalSemaphoreCount = static_cast<uint32_t> (signalSemaphores.size ());
    result.pSignalSemaphores    = signalSemaphores.data ();

    vkQueueSubmit (handle, 1, &result, fenceToSignal);
}

}