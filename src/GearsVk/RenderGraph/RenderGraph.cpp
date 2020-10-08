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


RenderGraph::Pass RenderGraph::GetNextPass (const Pass& lastPass) const
{
    RenderGraph::Pass result;

    for (auto& op : lastPass.operations) {
        for (auto& res : op->GetPointingTo<Resource> ()) {
            for (auto& nextOp : res->GetPointingTo<Operation> ()) {
                result.operations.insert (nextOp);
            }
        }
    }

    for (auto& op : result.operations) {
        auto inputs  = op->GetPointingHere<Resource> ();
        auto outputs = op->GetPointingTo<Resource> ();
        result.inputs.insert (inputs.begin (), inputs.end ());
        result.outputs.insert (outputs.begin (), outputs.end ());
    }

    return result;
}


RenderGraph::Pass RenderGraph::GetFirstPass () const
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

    RenderGraph::Pass actualResult;
    actualResult.operations = result;

    for (auto& op : actualResult.operations) {
        auto inputs  = op->GetPointingHere<Resource> ();
        auto outputs = op->GetPointingTo<Resource> ();
        actualResult.inputs.insert (inputs.begin (), inputs.end ());
        actualResult.outputs.insert (outputs.begin (), outputs.end ());
    }

    return actualResult;
}


std::vector<RenderGraph::Pass> RenderGraph::GetPasses () const
{
    std::vector<Pass> result;

    Pass nextPass = GetFirstPass ();
    do {
        result.push_back (nextPass);
        nextPass = GetNextPass (nextPass);
    } while (!nextPass.operations.empty ());

    return result;
}


void RenderGraph::Compile (const GraphSettings& settings)
{
    compileSettings = settings;

    settings.GetDevice ().Wait ();
    vkQueueWaitIdle (settings.GetDevice ().GetGraphicsQueue ());

    passes = GetPasses ();

    CompileResources (settings);


    for (Pass& pass : passes) {
        for (Operation* op : pass.operations) {
            ImageResource* firstImgRes = nullptr;

            for (Resource* res : pass.outputs) {
                if (ImageResource* imgres = dynamic_cast<ImageResource*> (res)) {
                    if (firstImgRes == nullptr) {
                        firstImgRes = imgres;
                    } else {
                        const uint32_t firstWidth  = firstImgRes->GetImages ()[0]->GetWidth ();
                        const uint32_t firstHeight = firstImgRes->GetImages ()[0]->GetHeight ();

                        const uint32_t currentWidth  = imgres->GetImages ()[0]->GetWidth ();
                        const uint32_t currentHeight = imgres->GetImages ()[0]->GetHeight ();

                        if (firstWidth != currentWidth || firstHeight != currentHeight) {
                            throw std::runtime_error ("inconsistent output image extents");
                        }
                    }
                }
            }

            if (GVK_ERROR (firstImgRes == nullptr)) {
                op->Compile (settings, 500, 500);
            } else {
                op->Compile (settings, firstImgRes->GetImages ()[0]->GetWidth (), firstImgRes->GetImages ()[0]->GetHeight ());
            }
        }
    }

    operationCommandBuffers.clear ();
    resourceReadCommandBuffers.clear ();
    resourceWriteCommandBuffers.clear ();

    for (Pass& p : passes) {
        for (Operation* op : p.operations) {
            std::vector<CommandBufferP> commandBuffers;

            for (uint32_t frameIndex = 0; frameIndex < settings.framesInFlight; ++frameIndex) {
                auto opCmdBuf = CommandBuffer::CreateShared (settings.GetDevice ());
                opCmdBuf->Begin ();
                op->Record (frameIndex, *opCmdBuf);
                opCmdBuf->CmdPipelineBarrier (VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
                opCmdBuf->End ();
                commandBuffers.push_back (opCmdBuf);
            }

            operationCommandBuffers.insert ({ op->GetUUID (), std::move (commandBuffers) });
        }

        for (Resource* input : p.inputs) {
            if (auto imgInput = dynamic_cast<ImageResource*> (input)) {
                std::vector<CommandBufferP> commandBuffers;

                for (uint32_t frameIndex = 0; frameIndex < settings.framesInFlight; ++frameIndex) {
                    auto opCmdBuf = CommandBuffer::CreateShared (settings.GetDevice ());
                    opCmdBuf->Begin ();
                    imgInput->BindRead (frameIndex, *opCmdBuf);
                    opCmdBuf->End ();
                    commandBuffers.push_back (opCmdBuf);
                }

                resourceReadCommandBuffers.insert ({ imgInput->GetUUID (), std::move (commandBuffers) });
            }
        }

        for (Resource* output : p.outputs) {
            if (auto imgOutput = dynamic_cast<ImageResource*> (output)) {
                std::vector<CommandBufferP> commandBuffers;

                for (uint32_t frameIndex = 0; frameIndex < settings.framesInFlight; ++frameIndex) {
                    auto opCmdBuf = CommandBuffer::CreateShared (settings.GetDevice ());
                    opCmdBuf->Begin ();
                    imgOutput->BindWrite (frameIndex, *opCmdBuf);
                    opCmdBuf->End ();
                    commandBuffers.push_back (opCmdBuf);
                }

                resourceWriteCommandBuffers.insert ({ imgOutput->GetUUID (), std::move (commandBuffers) });
            }
        }
    }

    compiled = true;

    compileEvent ();
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

    std::vector<CommandBuffer*> submittedCommandBuffers;
    for (Pass& pass : passes) {
        for (Resource* res : pass.inputs) {
            if (auto imgOutput = dynamic_cast<ImageResource*> (res)) {
                submittedCommandBuffers.push_back (resourceReadCommandBuffers.at (res->GetUUID ()).at (frameIndex).get ());
            }
        }
        for (Resource* res : pass.outputs) {
            if (auto imgOutput = dynamic_cast<ImageResource*> (res)) {
                submittedCommandBuffers.push_back (resourceWriteCommandBuffers.at (res->GetUUID ()).at (frameIndex).get ());
            }
        }
        for (Operation* operation : pass.operations) {
            if (operation->IsActive ()) {
                submittedCommandBuffers.push_back (operationCommandBuffers.at (operation->GetUUID ()).at (frameIndex).get ());
            }
        }
    }

    // TODO
    std::vector<VkPipelineStageFlags> waitDstStageMasks (waitSemaphores.size (), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    //VkSubmitInfo result         = {};
    //result.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    //result.waitSemaphoreCount   = static_cast<uint32_t> (waitSemaphores.size ());
    //result.pWaitSemaphores      = waitSemaphores.data ();
    //result.pWaitDstStageMask    = waitDstStageMasks.data ();
    //result.commandBufferCount   = static_cast<uint32_t> (submittedCommandBuffers.size ());
    //result.pCommandBuffers      = submittedCommandBuffers.data ();
    //result.signalSemaphoreCount = static_cast<uint32_t> (signalSemaphores.size ());
    //result.pSignalSemaphores    = signalSemaphores.data ();

    //vkQueueSubmit (compileSettings.GetDevice ().GetGraphicsQueue (), 1, &result, fenceToSignal);

    compileSettings.GetDevice ().GetGraphicsQueue ().Submit (waitSemaphores, waitDstStageMasks, submittedCommandBuffers, signalSemaphores, fenceToSignal);
}


void RenderGraph::Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores)
{
    GVK_ASSERT (swapchain.SupportsPresenting ());

    // TODO itt present queue kene
    swapchain.Present (compileSettings.GetDevice ().GetGraphicsQueue (), imageIndex, waitSemaphores);
}

} // namespace RG
