#include "RenderGraph.hpp"

#include "RenderGraphCompileResult.hpp"

namespace RG {

RenderGraph::RenderGraph ()
    : compiled (false)
    , compileSettings ()
{
}


ResourceP RenderGraph::AddResource (ResourceP resource)
{
    compiled = false;

    resources.push_back (resource);

    return resource;
}


OperationP RenderGraph::AddOperation (OperationP operation)
{
    compiled = false;

    operations.push_back (operation);

    return operation;
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
    GVK_ASSERT (compiled);

    for (uint32_t frameIndex = 0; frameIndex < compileSettings.framesInFlight; ++frameIndex) {
        std::vector<uint32_t> operationsToRecompile = compileResult->GetOperationsToRecord (frameIndex, commandBufferIndex);

        // commandBufferIndex.ResetAll ();
        // commandBufferIndex.BeginAll ();
        // record operationsToRecompile
        // commandBufferIndex.EndAll ();
    }
}


std::set<Operation*> RenderGraph::GetNextOperations (const std::set<Operation*>& lastOperations) const
{
    std::set<Operation*> result;

    for (auto& op : lastOperations) {
        for (auto& res : op->GetPointingTo<Resource> ()) {
            for (auto& nextOp : res->GetPointingTo<Operation> ()) {
                result.insert (nextOp);
            }
        }
    }

    return result;
}


std::set<Operation*> RenderGraph::GetFirstPassOperations () const
{
    std::set<Operation*> result;

    for (auto& op : operations) {
        const std::vector<Resource*> opInputs = op->GetPointingHere<Resource> ();

        const bool allInputsAreFirstWrittenByThisOp = std::all_of (opInputs.begin (), opInputs.end (), [] (Resource* res) {
            return !res->HasPointingHere ();
        });

        if (allInputsAreFirstWrittenByThisOp || opInputs.empty ()) {
            result.insert (op.get ());
        }
    }
    return result;
}


std::vector<std::set<Operation*>> RenderGraph::GetPasses () const
{
    std::vector<std::set<Operation*>> result;

    std::set<Operation*> initialOperations = GetFirstPassOperations ();
    if (initialOperations.empty ()) {
        throw std::runtime_error ("bad graph layout");
    }
    result.push_back (initialOperations);

    std::set<Operation*> nextOperations = GetFirstPassOperations ();
    while (!nextOperations.empty ()) {
        result.push_back (nextOperations);
        nextOperations = GetNextOperations (nextOperations);
    }

    return result;
}


void RenderGraph::Compile (const GraphSettings& settings)
{
    compileSettings = settings;

    settings.GetDevice ().Wait ();
    vkQueueWaitIdle (settings.GetDevice ().GetGraphicsQueue ());

    auto passes = GetPasses ();

    try {
        CompileResources (settings);

        for (auto& op : operations) {
            op->Compile (settings);
        }

        compileResult.reset ();

        CompileResultU newCR = CompileResult::Create (settings.GetDevice (), settings.framesInFlight);

        newCR->BeginAll ();

        for (uint32_t frameIndex = 0; frameIndex < settings.framesInFlight; ++frameIndex) {
            uint32_t opIndex = 0;

            for (const std::set<Operation*>& pass : passes) {
                CommandBuffer& currentCommandBuffer = newCR->GetCommandBufferToRecord (frameIndex, opIndex);

                for (Operation* op : pass) {
                    for (Resource* inputResource : op->GetPointingHere<Resource> ()) {
                        if (auto imgRes = dynamic_cast<ImageResource*> (inputResource)) {
                            imgRes->BindRead (frameIndex, currentCommandBuffer);
                        }
                    }
                    for (Resource* outputResource : op->GetPointingTo<Resource> ()) {
                        if (auto imgRes = dynamic_cast<ImageResource*> (outputResource)) {
                            imgRes->BindWrite (frameIndex, currentCommandBuffer);
                        }
                    }
                    op->Record (frameIndex, currentCommandBuffer);
                }

                currentCommandBuffer.CmdPipelineBarrier (
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // srcStageMask
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT     // dstStageMask
                );

                ++opIndex;
            }
        }

        newCR->EndAll ();

        compileResult = std::move (newCR);
        compiled      = true;

    } catch (std::exception& ex) {
        GVK_ERROR (true);
        std::cout << ex.what () << std::endl;
        compiled = false;
    }
}


void RenderGraph::CreateInputConnection (Operation& op, Resource& res, InputBindingU&& conn)
{
    compiled = false;

    res.AddConnectionTo (op);

    op.AddInput (std::move (conn));
}


void RenderGraph::CreateOutputConnection (Operation& operation, uint32_t binding, ImageResource& resource)
{
    compiled = false;

    operation.AddConnectionTo (resource);

    operation.AddOutput (binding, resource);
}


void RenderGraph::Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkSemaphore>& signalSemaphores, VkFence fenceToSignal)
{
    if (GVK_ERROR (!compiled)) {
        return;
    }

    if (GVK_ERROR (frameIndex >= compileSettings.framesInFlight)) {
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

    vkQueueSubmit (compileSettings.GetDevice ().GetGraphicsQueue (), 1, &result, fenceToSignal);
}


void RenderGraph::Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores)
{
    GVK_ASSERT (swapchain.SupportsPresenting ());

    // TODO itt present queue kene
    swapchain.Present (compileSettings.GetDevice ().GetGraphicsQueue (), imageIndex, waitSemaphores);
}

} // namespace RG
