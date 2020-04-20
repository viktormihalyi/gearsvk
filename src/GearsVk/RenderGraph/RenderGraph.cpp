#include "RenderGraph.hpp"


namespace RG {

RenderGraph::RenderGraph (VkDevice device, VkCommandPool commandPool)
    : device (device)
    , commandPool (commandPool)
    , compiled (false)
    , compileSettings ()
{
}


Resource& RenderGraph::AddResource (Resource::U&& resource)
{
    compiled = false;

    resources.push_back (std::move (resource));
    return *resources[resources.size () - 1];
}


Operation& RenderGraph::AddOperation (Operation::U&& operation)
{
    compiled = false;

    operations.push_back (std::move (operation));
    return *operations[operations.size () - 1];
}


template<typename T>
bool Contains (const std::vector<T>& vec, const T& value)
{
    return std::find (vec.begin (), vec.end (), value) != vec.end ();
}


template<typename T>
bool Contains (const std::set<T>& vec, const T& value)
{
    return std::find (vec.begin (), vec.end (), value) != vec.end ();
}


void RenderGraph::CompileResources (const GraphSettings& settings)
{
    for (auto& res : resources) {
        res->Compile (settings);
    }
}


void RenderGraph::Recompile (uint32_t commandBufferIndex)
{
    ASSERT (compiled);

    for (uint32_t frameIndex = 0; frameIndex < compileSettings.framesInFlight; ++frameIndex) {
        std::vector<uint32_t> operationsToRecompile = compileResult->GetOperationsToRecord (frameIndex, commandBufferIndex);

        // commandBufferIndex.ResetAll ();
        // commandBufferIndex.BeginAll ();
        // record operationsToRecompile
        // commandBufferIndex.EndAll ();
    }
}


void RenderGraph::Compile (const GraphSettings& settings)
{
    compileSettings = settings;

    settings.GetDevice ().Wait ();
    vkQueueWaitIdle (settings.queue);

    try {
        for (auto& op : operations) {
            op->Compile (settings);
        }

        compileResult.reset ();

        CompileResult::U newCR = CompileResult::Create (settings.GetDevice (), settings.commandPool, settings.framesInFlight);

        newCR->BeginAll ();

        for (uint32_t frameIndex = 0; frameIndex < settings.framesInFlight; ++frameIndex) {
            uint32_t opIndex = 0;
            for (auto& op : operations) {
                CommandBuffer& currentCommandBuffer = newCR->GetCommandBufferToRecord (frameIndex, opIndex);

                for (auto& inputResource : op->inputs) {
                    inputResource.get ().BindRead (frameIndex, currentCommandBuffer);
                }
                for (auto& outputResource : op->outputs) {
                    outputResource.get ().BindWrite (frameIndex, currentCommandBuffer);
                }

                op->Record (frameIndex, currentCommandBuffer);

                if (&op != &operations.back ()) {
                    currentCommandBuffer.CmdPipelineBarrier (
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // srcStageMask
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT     // dstStageMask
                    );
                }

                ++opIndex;
            }
        }

        newCR->EndAll ();

        compileResult = std::move (newCR);
        compiled      = true;

    } catch (std::exception& ex) {
        ERROR (true);
        std::cout << ex.what () << std::endl;
        compiled = false;
    }
}


void RenderGraph::AddConnection (const RenderGraph::InputConnection& c)
{
    compiled = false;

    c.operation.AddInput (c.binding, c.resource);
}


void RenderGraph::AddConnection (const RenderGraph::OutputConnection& c)
{
    compiled = false;

    c.operation.AddOutput (c.binding, c.resource);
}


void RenderGraph::Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkSemaphore>& signalSemaphores, VkFence fenceToSignal)
{
    if (ERROR (!compiled)) {
        return;
    }

    if (ERROR (frameIndex >= compileSettings.framesInFlight)) {
        return;
    }

    std::vector<VkCommandBuffer> cmdHdl = compileResult->GetCommandBuffersToSubmit (frameIndex);

    // TODO
    std::vector<VkPipelineStageFlags> waitDstStageMasks (waitSemaphores.size (), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    VkSubmitInfo result         = {};
    result.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    result.waitSemaphoreCount   = waitSemaphores.size ();
    result.pWaitSemaphores      = waitSemaphores.data ();
    result.pWaitDstStageMask    = waitDstStageMasks.data ();
    result.commandBufferCount   = cmdHdl.size ();
    result.pCommandBuffers      = cmdHdl.data ();
    result.signalSemaphoreCount = signalSemaphores.size ();
    result.pSignalSemaphores    = signalSemaphores.data ();

    vkQueueSubmit (compileSettings.queue, 1, &result, fenceToSignal);
}

} // namespace RG
