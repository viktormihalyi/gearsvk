#include "RenderGraph.hpp"


namespace RenderGraph {

Graph::Graph (VkDevice device, VkCommandPool commandPool, GraphSettings settings)
    : device (device)
    , commandPool (commandPool)
    , settings (settings)
{
}


Resource& Graph::CreateResource (Resource::U&& resource)
{
    resources.push_back (std::move (resource));
    return *resources[resources.size () - 1];
}


Operation& Graph::CreateOperation (Operation::U&& resource)
{
    operations.push_back (std::move (resource));
    return *operations[operations.size () - 1];
}


void Graph::Compile ()
{
    for (auto& op : operations) {
        op->Compile ();
    }

    commandBuffers.resize (settings.framesInFlight);

    for (uint32_t frameIndex = 0; frameIndex < settings.framesInFlight; ++frameIndex) {
        CommandBuffer::U& currentCommandBuffer = commandBuffers[frameIndex];

        currentCommandBuffer = CommandBuffer::Create (device, commandPool);
        currentCommandBuffer->Begin ();
        for (auto& op : operations) {
            for (auto& inputResource : op->inputs) {
                inputResource.get ().BindRead (frameIndex, *currentCommandBuffer);
            }
            for (auto& outputResource : op->outputs) {
                outputResource.get ().BindWrite (frameIndex, *currentCommandBuffer);
            }

            op->Record (frameIndex, *currentCommandBuffer);

            if (&op != &operations.back ()) {
                VkMemoryBarrier memoryBarrier = {};
                memoryBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                memoryBarrier.srcAccessMask   = VK_ACCESS_SHADER_WRITE_BIT;
                memoryBarrier.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier (
                    *currentCommandBuffer,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // srcStageMask
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,    // dstStageMask
                    0,
                    0, nullptr, //&memoryBarrier, // memory barriers
                    0, nullptr, // buffer memory barriers
                    0, nullptr  // image barriers
                );
            }
        }
        currentCommandBuffer->End ();
    }
}


void Graph::Submit (VkQueue queue, uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkSemaphore>& signalSemaphores)
{
    if (ERROR (frameIndex >= settings.framesInFlight)) {
        return;
    }

    VkCommandBuffer cmdHdl = *commandBuffers[frameIndex];

    // TODO
    std::vector<VkPipelineStageFlags> waitDstStageMasks (3, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

    VkSubmitInfo result         = {};
    result.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    result.waitSemaphoreCount   = waitSemaphores.size ();
    result.pWaitSemaphores      = waitSemaphores.data ();
    result.pWaitDstStageMask    = waitDstStageMasks.data ();
    result.commandBufferCount   = 1;
    result.pCommandBuffers      = &cmdHdl;
    result.signalSemaphoreCount = signalSemaphores.size ();
    result.pSignalSemaphores    = signalSemaphores.data ();

    vkQueueSubmit (queue, 1, &result, nullptr);

    for (auto& op : operations) {
        op->OnPostSubmit (frameIndex);
    }
}

} // namespace RenderGraph
