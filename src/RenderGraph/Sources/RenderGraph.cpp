#include "RenderGraph.hpp"

#include "GraphSettings.hpp"
#include "Operation.hpp"
#include "DrawRecordable.hpp"
#include "Resource.hpp"
#include "Utils/CommandLineFlag.hpp"

#include "spdlog/spdlog.h"

namespace GVK {

namespace RG {

RenderGraph::RenderGraph ()
    : compiled (false)
{
}


template<typename T>
bool Contains (const std::vector<T>& vec, const T& value)
{
    return std::find (vec.begin (), vec.end (), value) != vec.end ();
}


void RenderGraph::CompileResources (const GraphSettings& settings)
{
    Utils::ForEachP<Resource> (settings.connectionSet.insertionOrder, [&] (std::shared_ptr<Resource>& res) {
        res->Compile (settings);
    });
}


Pass RenderGraph::GetNextPass (const ConnectionSet& connectionSet, const Pass& lastPass) const
{
    Pass result;

    for (Operation* op : lastPass.GetAllOperations ()) {
        for (const std::shared_ptr<Resource>& res : connectionSet.GetPointingTo<Resource> (op)) {
            for (const std::shared_ptr<Operation>& nextOp : connectionSet.GetPointingTo<Operation> (res.get ())) {
                result.AddInput (nextOp.get (), res.get ());
            }
        }
    }

    for (auto& op : result.GetAllOperations ()) {
        std::vector<std::shared_ptr<Resource>> allInputs  = connectionSet.GetPointingHere<Resource> (op);
        std::vector<std::shared_ptr<Resource>> allOutputs = connectionSet.GetPointingTo<Resource> (op);
        for (std::shared_ptr<Resource> input : allInputs) {
            result.AddInput (op, input.get ());
        }
        for (std::shared_ptr<Resource> output : allOutputs) {
            result.AddOutput (op, output.get ());
        }
    }

    return result;
}


Pass RenderGraph::GetFirstPass (const ConnectionSet& connectionSet) const
{
    std::set<Operation*> allOpSet;
    std::vector<Operation*> allOp;

    Utils::ForEachP<Operation> (connectionSet.insertionOrder, [&] (const std::shared_ptr<Operation>& op) {
        const std::vector<std::shared_ptr<Resource>> opInputs = connectionSet.GetPointingHere<Resource> (op.get ());

        const bool allInputsAreFirstWrittenByThisOp = std::all_of (opInputs.begin (), opInputs.end (), [&] (const std::shared_ptr<Resource>& res) {
            return connectionSet.GetPointingHere<Node> (res.get ()).empty ();
        });

        if (allInputsAreFirstWrittenByThisOp || opInputs.empty ())
            if (allOpSet.insert (op.get ()).second)
                allOp.push_back (op.get ());
    });

    Pass actualResult;

    for (Operation* op : allOp) {
        std::vector<std::shared_ptr<Resource>> allInputs  = connectionSet.GetPointingHere<Resource> (op);
        std::vector<std::shared_ptr<Resource>> allOutputs = connectionSet.GetPointingTo<Resource> (op);
        for (std::shared_ptr<Resource> input : allInputs) {
            actualResult.AddInput (op, input.get ());
        }
        for (std::shared_ptr<Resource> output : allOutputs) {
            actualResult.AddOutput (op, output.get ());
        }
    }

    return actualResult;
}


std::vector<Pass> RenderGraph::GetPasses (const ConnectionSet& connectionSet) const
{
    std::vector<Pass> result;

    Pass nextPass = GetFirstPass (connectionSet);
    do {
        result.push_back (nextPass);
        nextPass = GetNextPass (connectionSet, nextPass);
    } while (!nextPass.GetAllOperations ().empty ());

    return result;
}

struct OutputHitCount {
    std::unordered_map<Resource*, std::vector<Operation*>> hitCount;
    std::vector<Resource*> insertionOrder;
};

static OutputHitCount GetOutputHitCount (Pass pass)
{
    const std::vector<Operation*> operations = pass.GetAllOperations ();

    OutputHitCount result;

    for (const auto op : operations) {
        const auto opIO = pass.GetOperationIO (op);
        for (Resource* output : opIO->outputs) {
            const bool contains = result.hitCount.find (output) != result.hitCount.end ();
            result.hitCount[output].push_back (op);
            if (!contains) {
                result.insertionOrder.push_back (output);
            }
        }
    }

    return result;
}


static bool HasMultipleOperationsToOneOutput (Pass pass)
{
    const OutputHitCount  outputCount = GetOutputHitCount (pass);
    
    for (const auto& oc : outputCount.hitCount) {
        if (oc.second.size () > 1) {
            return true;
        }
    }

    return false;
}


void RenderGraph::SeparatePasses (const ConnectionSet& connectionSet)
{
    std::vector<Pass> newPasses { passes };
    
    bool seperationHappened = false;

    for (size_t i = 0; i < newPasses.size (); ++i) {

        const bool needsSeperation = HasMultipleOperationsToOneOutput (newPasses[i]);
        if (needsSeperation) {
            const OutputHitCount hitCount = GetOutputHitCount (newPasses[i]);

            if (i == newPasses.size () - 1) {
                newPasses.emplace_back ();
            }

            Resource* firstResourceToSplitOn = hitCount.insertionOrder[0];
            std::vector<Operation*> operationsToSplit = hitCount.hitCount.find (firstResourceToSplitOn)->second;

            Operation* firstOperation = operationsToSplit[0];
            Operation* secondOperation = operationsToSplit[1];
            
            Pass::OperationIO* toMove = newPasses[i].GetOperationIO (secondOperation);

            newPasses[i + 1].AddOperationIO (toMove);
            newPasses[i].RemoveOperationIO (toMove);

            seperationHappened = true;
            break;
        }
    }
    
    passes = newPasses;


    if (seperationHappened)
        SeparatePasses (connectionSet);
}


Utils::CommandLineOnOffFlag printRenderGraphFlag { "--printRenderGraph", "Prints render graph passes, operatins, resources." };


class ImageResourceLayoutState {
private:
    std::shared_ptr<Resource>    resource;
    std::optional<VkImageLayout> layout;

public:
};


void RenderGraph::Compile (GraphSettings&& settings)
{
    settings.GetDevice ().Wait ();
    vkQueueWaitIdle (settings.GetDevice ().GetGraphicsQueue ());

    passes = GetPasses (settings.connectionSet);

    SeparatePasses (settings.connectionSet);

    if (printRenderGraphFlag.IsFlagOn ()) {
        std::stringstream logString;
        for (size_t i = 0; i < passes.size (); ++i) {
            const Pass& pass = passes[i];
            logString << "Pass " << i << std::endl;
            for (const Operation* op : pass.GetAllOperations ()) {
                logString << "\tOperation \"" << op->GetName () << "\" (desc: \"" << op->GetDescription () << "\", " << op->GetUUID ().GetValue () << ")" << std::endl;
                logString << "\tInputs:" << std::endl;
                auto resinp = settings.connectionSet.GetPointingHere<Resource> (op);
                for (auto res : resinp) {
                    logString << "\t\tInput Resource \"" << res->GetName () << "\" (desc: \"" << res->GetDescription () << "\", id: " << res->GetUUID ().GetValue () << ")" << std::endl;
                }
                logString << "\tOutputs:" << std::endl;
                auto resout = settings.connectionSet.GetPointingTo<Resource> (op);
                for (auto res : resout) {
                    logString << "\t\tOutput Resource \"" << res->GetName () << "\" (desc: \"" << res->GetDescription () << "\", id: " << res->GetUUID ().GetValue () << ")" << std::endl;
                }
            }
        } 

        std::cout << "======= Render graph begin =======" << std::endl;
        std::cout << logString.str () << std::endl;
        std::cout << "======= Render graph end =========" << std::endl;
    }

    CompileResources (settings);


    for (Pass& pass : passes) {
        for (Operation* op : pass.GetAllOperations ()) {
            ImageResource* firstImgRes = nullptr;

            for (Resource* res : pass.GetAllOutputs ()) {
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

    imageLayoutSequence.clear ();

    for (Pass& p : passes) {
        Utils::ForEach<ImageResource*> (p.GetAllInputs (), [&] (ImageResource* img) {
            for (auto i : img->GetImages ()) {
                imageLayoutSequence[i].push_back (img->GetInitialLayout ());
            }
        });
        Utils::ForEach<ImageResource*> (p.GetAllOutputs (), [&] (ImageResource* img) {
            for (auto i : img->GetImages ()) {
                imageLayoutSequence[i].push_back (img->GetInitialLayout ());
            }
        });
    }

    VkMemoryBarrier flushAllMemory = {};
    flushAllMemory.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    flushAllMemory.srcAccessMask   = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
                                   VK_ACCESS_INDEX_READ_BIT |
                                   VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                                   VK_ACCESS_UNIFORM_READ_BIT |
                                   VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_SHADER_READ_BIT |
                                   VK_ACCESS_SHADER_WRITE_BIT |
                                   VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                   VK_ACCESS_TRANSFER_READ_BIT |
                                   VK_ACCESS_TRANSFER_WRITE_BIT;
    flushAllMemory.dstAccessMask = flushAllMemory.srcAccessMask;

    commandBuffers.clear ();
    commandBuffers2.clear ();

    for (uint32_t frameIndex = 0; frameIndex < settings.framesInFlight; ++frameIndex) {
        commandBuffers.emplace_back (settings.GetDevice ());

        std::vector<CommandBuffer> commandBuffersForPasses;
        for (size_t i = 0; i < passes.size (); ++i) {
            commandBuffersForPasses.emplace_back (settings.GetDevice ());
        }
        for (size_t i = 0; i < commandBuffersForPasses.size (); ++i) {
            commandBuffersForPasses[i].Begin ();
        }

        CommandBuffer& currentCmdbuffer = commandBuffers[frameIndex];

        currentCmdbuffer.Begin ();

        /*
        for (Pass& p : passes) {
            for (auto res : p.GetAllInputs ()) {
                res->OnGraphExecutionStarted (frameIndex, currentCmdbuffer);
            }
            for (auto res : p.GetAllOutputs ()) {
                res->OnGraphExecutionStarted (frameIndex, currentCmdbuffer);
            }
        }
        */

        size_t currentPass = 0;

        for (Pass& p : passes) {
            CommandBuffer& currentCmdbuffer2 = commandBuffersForPasses[currentPass++];

            for (auto op : p.GetAllOperations ()) {
                auto allInputs  = settings.connectionSet.GetPointingHere<Resource> (op);
                auto allOutputs = settings.connectionSet.GetPointingTo<Resource> (op);
                /*
                for (auto res : allInputs) {
                    res->OnPreRead (frameIndex, currentCmdbuffer);
                }

                for (auto res : allOutputs) {
                    res->OnPreWrite (frameIndex, currentCmdbuffer);
                }
                */

                std::vector<VkImageMemoryBarrier> imgBarriers;

                Utils::ForEachP<ImageResource> (allInputs, [&] (const std::shared_ptr<ImageResource>& img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        const VkImageLayout currentLayout = imageLayoutSequence[imgbase].back ();
                        const VkImageLayout newLayout     = op->GetImageLayoutAtStartForInputs (*img);
                        //if (newLayout != currentLayout) {
                        imgBarriers.push_back (imgbase->GetBarrier (currentLayout, newLayout, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT));
                        imageLayoutSequence[imgbase].push_back (newLayout);
                        //}
                    }
                });

                Utils::ForEachP<ImageResource> (allOutputs, [&] (const std::shared_ptr<ImageResource>& img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        const VkImageLayout currentLayout = imageLayoutSequence[imgbase].back ();
                        const VkImageLayout newLayout     = op->GetImageLayoutAtStartForOutputs (*img);
                        //if (newLayout != currentLayout) {
                        imgBarriers.push_back (imgbase->GetBarrier (currentLayout, newLayout, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT));
                        imageLayoutSequence[imgbase].push_back (newLayout);
                        // }
                    }
                });

                for (auto& img : imgBarriers) {
                    img.srcAccessMask = flushAllMemory.srcAccessMask;
                    img.dstAccessMask = flushAllMemory.dstAccessMask;
                }

                currentCmdbuffer.Record<CommandPipelineBarrier> (
                                    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                    std::vector<VkMemoryBarrier> { flushAllMemory },
                                    std::vector<VkBufferMemoryBarrier> {},
                                    imgBarriers)
                    .SetName ("Transition for next Pass");

                currentCmdbuffer2.Record<CommandPipelineBarrier> (
                                    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                    std::vector<VkMemoryBarrier> { flushAllMemory },
                                    std::vector<VkBufferMemoryBarrier> {},
                                    imgBarriers)
                    .SetName ("Transition for next Pass");

                Utils::ForEachP<ImageResource> (allInputs, [&] (const std::shared_ptr<ImageResource>& img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        imageLayoutSequence[imgbase].push_back (op->GetImageLayoutAtEndForInputs (*img));
                    }
                });

                Utils::ForEachP<ImageResource> (allOutputs, [&] (const std::shared_ptr<ImageResource>& img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        imageLayoutSequence[imgbase].push_back (op->GetImageLayoutAtEndForOutputs (*img));
                    }
                });
            }

            for (auto op : p.GetAllOperations ()) {
                op->Record (settings.connectionSet, frameIndex, currentCmdbuffer);
            }

            for (auto op : p.GetAllOperations ()) {
                op->Record (settings.connectionSet, frameIndex, currentCmdbuffer2);
            }
            /*
            for (auto op : p.GetAllOperations ()) {
                auto allInputs  = settings.connectionSet.GetPointingHere<Resource> (op);
                auto allOutputs = settings.connectionSet.GetPointingTo<Resource> (op);
                for (auto res : p.GetAllOutputs ()) {
                    res->OnPostWrite (frameIndex, currentCmdbuffer);
                }
            }
            */

        currentCmdbuffer2.Record<CommandPipelineBarrier> (
                                VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                std::vector<VkMemoryBarrier> { flushAllMemory },
                                std::vector<VkBufferMemoryBarrier> {},
                                 std::vector<VkImageMemoryBarrier> {})
                .SetName ("Transition to initial layout");

            currentCmdbuffer2.End ();

        }

        commandBuffers2.push_back (std::move (commandBuffersForPasses));

        std::vector<VkImageMemoryBarrier> transitionToInitial;
        for (Pass& p : passes) {
            Utils::ForEach<ImageResource*> (p.GetAllInputs (), [&] (ImageResource* img) {
                for (auto imgbase : img->GetImages (frameIndex)) {
                    const VkImageLayout currentLayout = imageLayoutSequence[imgbase].back ();
                    //if (currentLayout != img->GetInitialLayout ()) {
                        transitionToInitial.push_back (imgbase->GetBarrier (currentLayout, img->GetInitialLayout (), VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT));
                    //}
                }
            });
        }

        for (auto& img : transitionToInitial) {
            img.srcAccessMask = flushAllMemory.srcAccessMask;
            img.dstAccessMask = flushAllMemory.dstAccessMask;
        }

        currentCmdbuffer.Record<CommandPipelineBarrier> (
                            VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                            VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
            std::vector<VkMemoryBarrier> { flushAllMemory},
            std::vector<VkBufferMemoryBarrier> {},
            transitionToInitial).SetName ("Transition to initial layout");
        
        /*
        for (Pass& p : passes) {
            for (auto res : p.GetAllInputs ()) {
                res->OnGraphExecutionEnded (frameIndex, currentCmdbuffer);
            }
            for (auto res : p.GetAllOutputs ()) {
                res->OnGraphExecutionEnded (frameIndex, currentCmdbuffer);
            }
        }
        */

        currentCmdbuffer.End ();
    }

    graphSettings = std::move (settings);

    compiled = true;
}


void RenderGraph::Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkSemaphore>& signalSemaphores, VkFence fenceToSignal)
{
    if (GVK_ERROR (!compiled)) {
        return;
    }

    if (GVK_ERROR (frameIndex >= graphSettings.framesInFlight)) {
        return;
    }


    // TODO
    std::vector<VkPipelineStageFlags> waitDstStageMasks (waitSemaphores.size (), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    graphSettings.device->GetGraphicsQueue ().Submit (waitSemaphores, waitDstStageMasks, { &commandBuffers[frameIndex] }, signalSemaphores, fenceToSignal);
}


void RenderGraph::Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores)
{
    GVK_ASSERT (swapchain.SupportsPresenting ());

    // TODO itt present queue kene
    swapchain.Present (graphSettings.device->GetGraphicsQueue (), imageIndex, waitSemaphores);
}


uint32_t RenderGraph::GetPassCount () const
{
    GVK_ASSERT (compiled);

    return passes.size ();
}

} // namespace RG

} // namespace GVK
