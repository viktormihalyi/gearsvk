#include "RenderGraph.hpp"


namespace RenderGraphns {

RenderGraph::RenderGraph (VkDevice device, VkCommandPool commandPool, GraphSettings settings)
    : device (device)
    , commandPool (commandPool)
    , settings (settings)
    , compiled (false)
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


void RenderGraph::Compile ()
{
    /*
    std::set<Operation::Ref> startingOperations;
    // gather starting operations
    {
        for (const auto& o : operations) {
            if (o->inputs.empty ()) {
                startingOperations.insert (*o);
            }
        }

        std::set<Resource::Ref> onlyReadResources;
        for (const auto& r : resources) {
            bool isResourceWritten = false;
            for (const auto& o : operations) {
                for (const auto& outputRes : o->outputs) {
                    if (&outputRes.get () == r.get ()) {
                        isResourceWritten = true;
                        break;
                    }
                }
            }

            if (!isResourceWritten) {
                onlyReadResources.insert (*r);
            }
        }

        for (const auto& o : operations) {
            bool inputsResourcesAreNeverWritten = true;
            for (const auto& inputRes : o->inputs) {
                if (!Contains (onlyReadResources, inputRes)) {
                    inputsResourcesAreNeverWritten = false;
                    break;
                }
            }
            if (inputsResourcesAreNeverWritten) {
                startingOperations.insert (*o);
            }
        }
    }
    */


    try {
        compileResult.Clear ();

        for (auto& res : resources) {
            res->Compile (settings);
        }

        for (auto& op : operations) {
            op->Compile (settings);
        }

        compileResult.commandBuffers.resize (settings.framesInFlight);

        for (uint32_t frameIndex = 0; frameIndex < settings.framesInFlight; ++frameIndex) {
            CommandBuffer::U& currentCommandBuffer = compileResult.commandBuffers[frameIndex];

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
                    vkCmdPipelineBarrier (
                        *currentCommandBuffer,
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // srcStageMask
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,    // dstStageMask
                        0,
                        0, nullptr, // memory barriers
                        0, nullptr, // buffer memory barriers
                        0, nullptr  // image barriers
                    );
                }
            }
            currentCommandBuffer->End ();
        }

        compiled = true;

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

    if (ERROR (frameIndex >= settings.framesInFlight)) {
        return;
    }

    VkCommandBuffer cmdHdl = *compileResult.commandBuffers[frameIndex];

    // TODO
    std::vector<VkPipelineStageFlags> waitDstStageMasks (waitSemaphores.size (), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    VkSubmitInfo result         = {};
    result.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    result.waitSemaphoreCount   = waitSemaphores.size ();
    result.pWaitSemaphores      = waitSemaphores.data ();
    result.pWaitDstStageMask    = waitDstStageMasks.data ();
    result.commandBufferCount   = 1;
    result.pCommandBuffers      = &cmdHdl;
    result.signalSemaphoreCount = signalSemaphores.size ();
    result.pSignalSemaphores    = signalSemaphores.data ();

    vkQueueSubmit (settings.queue, 1, &result, fenceToSignal);
}

} // namespace RenderGraphns
