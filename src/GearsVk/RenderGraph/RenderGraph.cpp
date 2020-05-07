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
        CompileResources (settings);

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

                for (Resource& inputResource : op->inputs) {
                    if (auto imgRes = dynamic_cast<ImageResource*> (&inputResource)) {
                        imgRes->BindRead (frameIndex, currentCommandBuffer);
                    }
                }
                for (Resource& outputResource : op->outputs) {
                    if (auto imgRes = dynamic_cast<ImageResource*> (&outputResource)) {
                        imgRes->BindWrite (frameIndex, currentCommandBuffer);
                    }
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


void RenderGraph::CreateOutputConnection (Operation& operation, uint32_t binding, ImageResource& resource)
{
    compiled = false;

    operation.AddOutput (binding, resource);
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
    result.waitSemaphoreCount   = static_cast<uint32_t> (waitSemaphores.size ());
    result.pWaitSemaphores      = waitSemaphores.data ();
    result.pWaitDstStageMask    = waitDstStageMasks.data ();
    result.commandBufferCount   = static_cast<uint32_t> (cmdHdl.size ());
    result.pCommandBuffers      = cmdHdl.data ();
    result.signalSemaphoreCount = static_cast<uint32_t> (signalSemaphores.size ());
    result.pSignalSemaphores    = signalSemaphores.data ();

    vkQueueSubmit (compileSettings.queue, 1, &result, fenceToSignal);
}

} // namespace RG
