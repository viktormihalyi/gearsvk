#include "RenderGraph.hpp"


namespace RenderGraph {

Graph::Graph (VkDevice device, VkCommandPool commandPool, uint32_t framesInFlight, uint32_t width, uint32_t height)
    : device (device)
    , commandPool (commandPool)
    , framesInFlight (framesInFlight)
    , width (width)
    , height (height)
{
}


GraphInfo Graph::GetGraphInfo () const
{
    GraphInfo g;
    g.width  = width;
    g.height = height;
    return g;
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

    commandBuffers.resize (framesInFlight);

    for (uint32_t frameIndex = 0; frameIndex < framesInFlight; ++frameIndex) {
        CommandBuffer::U& currentCommandBuffer = commandBuffers[frameIndex];

        currentCommandBuffer = CommandBuffer::Create (device, commandPool);
        currentCommandBuffer->Begin ();
        for (auto& op : operations) {
            for (auto& inputResource : op->inputs) {
                inputResource.get ().BindRead (*currentCommandBuffer);
            }
            for (auto& outputResource : op->outputs) {
                outputResource.get ().BindWrite (*currentCommandBuffer);
            }

            op->Record (*currentCommandBuffer);

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
    if (ERROR (frameIndex >= framesInFlight)) {
        return;
    }

    VkCommandBuffer cmdHdl = *commandBuffers[frameIndex];

    VkSubmitInfo result         = {};
    result.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    result.waitSemaphoreCount   = 0;
    result.pWaitSemaphores      = nullptr;
    result.pWaitDstStageMask    = 0;
    result.commandBufferCount   = 1;
    result.pCommandBuffers      = &cmdHdl;
    result.signalSemaphoreCount = 0;
    result.pSignalSemaphores    = nullptr;
    vkQueueSubmit (queue, 1, &result, nullptr);

    //std::vector<VkSubmitInfo> submitInfos;
    //
    //for (auto& a : operations) {
    //    submitInfos.push_back (a->GetSubmitInfo ());
    //}
    //vkQueueSubmit (queue, submitInfos.size (), submitInfos.data (), VK_NULL_HANDLE);
    //vkQueueWaitIdle (queue);
}

} // namespace RenderGraph
