#include "RenderGraph.hpp"


namespace RenderGraph {


Graph::Graph (VkDevice device, VkCommandPool commandPool, uint32_t width, uint32_t height)
    : device (device)
    , commandPool (commandPool)
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


Resource::Ref Graph::CreateResource (Resource::U&& resource)
{
    resources.push_back (std::move (resource));
    return *resources[resources.size () - 1];
}


Operation::Ref Graph::CreateOperation (Operation::U&& resource)
{
    operations.push_back (std::move (resource));
    return *operations[operations.size () - 1];
}


void Graph::Compile ()
{
    for (auto& op : operations) {
        op->Compile ();
    }

    cmdBuf = CommandBuffer::Create (device, commandPool);
    cmdBuf->Begin ();
    for (auto& op : operations) {
        for (auto& inputResource : op->inputs) {
            inputResource.get ().BindRead (*cmdBuf);
        }
        for (auto& outputResource : op->outputs) {
            outputResource.get ().BindWrite (*cmdBuf);
        }

        op->Record (*cmdBuf);

        if (&op != &operations.back ()) {
            VkMemoryBarrier memoryBarrier = {};
            memoryBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            memoryBarrier.srcAccessMask   = VK_ACCESS_SHADER_WRITE_BIT;
            memoryBarrier.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier (
                *cmdBuf,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // srcStageMask
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,    // dstStageMask
                0,
                0, nullptr, //&memoryBarrier, // memory barriers
                0, nullptr, // buffer memory barriers
                0, nullptr  // image barriers
            );
        }
    }
    cmdBuf->End ();
}


void Graph::Submit (VkQueue queue)
{
    VkCommandBuffer cmdHdl = *cmdBuf;

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
