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
    , device (nullptr)
    , framesInFlight (0)
{
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
    Utils::ForEachP<Resource> (settings.connectionSet.nodes, [&] (std::shared_ptr<Resource>& res) {
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
    std::set<Operation*> allOp;

    Utils::ForEachP<Operation> (connectionSet.nodes, [&] (const std::shared_ptr<Operation>& op) {
        const std::vector<std::shared_ptr<Resource>> opInputs = connectionSet.GetPointingHere<Resource> (op.get ());

        const bool allInputsAreFirstWrittenByThisOp = std::all_of (opInputs.begin (), opInputs.end (), [&] (const std::shared_ptr<Resource>& res) {
            return connectionSet.GetPointingHere<Node> (res.get ()).empty ();
        });

        if (allInputsAreFirstWrittenByThisOp || opInputs.empty ()) {
            allOp.insert (op.get ());
        }
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



void RenderGraph::SeparatePasses (const ConnectionSet& connectionSet)
{
    std::vector<Pass> newPasses;
    
    const std::vector<Pass> oldPasses { passes };
    for (const Pass& p : oldPasses) {
        std::unordered_map<std::shared_ptr<Resource>, std::vector<Operation*>> operationOutputs;
        for (Operation* o : p.GetAllOperations ()) {
            for (auto& res : connectionSet.GetPointingTo<Resource> (o)) {
                operationOutputs[res].push_back (o);
            }
        }
        for (auto& [res, op] : operationOutputs) {
            if (op.size () > 1) {
                for (Operation* sepOp : op) {
                    Pass newSep;
                    for (auto inp : connectionSet.GetPointingHere<Resource> (sepOp)) {
                        newSep.AddInput (sepOp, inp.get ());
                    }
                    for (auto out : connectionSet.GetPointingTo<Resource> (sepOp)) {
                        newSep.AddOutput (sepOp, out.get ());
                    }
                    newPasses.push_back (newSep);
                }
            } else {
                newPasses.push_back (p);
            }
        }
    }

    passes = newPasses;
}


Utils::CommandLineOnOffFlag printRenderGraphFlag { "--printRenderGraph", "Prints render graph passes, operatins, resources." };


void RenderGraph::Compile (GraphSettings&& settings)
{
    device         = settings.device;
    framesInFlight = settings.framesInFlight;

    settings.GetDevice ().Wait ();
    vkQueueWaitIdle (settings.GetDevice ().GetGraphicsQueue ());

    passes = GetPasses (settings.connectionSet);

    if constexpr (false)
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

    std::unordered_map<Image*, VkImageLayout> layoutMap;

    for (Pass& p : passes) {
        Utils::ForEach<ImageResource*> (p.GetAllInputs (), [&] (ImageResource* img) {
            for (auto i : img->GetImages ()) {
                layoutMap[i] = img->GetInitialLayout ();
            }
        });
        Utils::ForEach<ImageResource*> (p.GetAllOutputs (), [&] (ImageResource* img) {
            for (auto i : img->GetImages ()) {
                layoutMap[i] = img->GetInitialLayout ();
            }
        });
    }

    c.clear ();
    for (uint32_t frameIndex = 0; frameIndex < settings.framesInFlight; ++frameIndex) {
        c.push_back (std::move (CommandBuffer { settings.GetDevice () }));

        c[frameIndex].Begin ();

        for (Pass& p : passes) {
            for (auto res : p.GetAllInputs ()) {
                res->OnGraphExecutionStarted (frameIndex, c[frameIndex]);
            }
            for (auto res : p.GetAllOutputs ()) {
                res->OnGraphExecutionStarted (frameIndex, c[frameIndex]);
            }
        }

        for (Pass& p : passes) {
            for (auto op : p.GetAllOperations ()) {
                auto allInputs  = settings.connectionSet.GetPointingHere<Resource> (op);
                auto allOutputs = settings.connectionSet.GetPointingTo<Resource> (op);

                for (auto res : allInputs) {
                    res->OnPreRead (frameIndex, c[frameIndex]);
                }

                for (auto res : allOutputs) {
                    res->OnPreWrite (frameIndex, c[frameIndex]);
                }

                std::vector<VkImageMemoryBarrier> imgBarriers;

                Utils::ForEachP<ImageResource> (allInputs, [&] (const std::shared_ptr<ImageResource>& img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        const VkImageLayout currentLayout = layoutMap[imgbase];
                        const VkImageLayout newLayout     = op->GetImageLayoutAtStartForInputs (*img);
                        if (newLayout != currentLayout) {
                            imgBarriers.push_back (imgbase->GetBarrier (currentLayout, newLayout));
                            layoutMap[imgbase] = newLayout;
                        }
                    }
                });

                Utils::ForEachP<ImageResource> (allOutputs, [&] (const std::shared_ptr<ImageResource>& img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        const VkImageLayout currentLayout = layoutMap[imgbase];
                        const VkImageLayout newLayout     = op->GetImageLayoutAtStartForOutputs (*img);
                        if (newLayout != currentLayout) {
                            imgBarriers.push_back (imgbase->GetBarrier (currentLayout, newLayout));
                            layoutMap[imgbase] = newLayout;
                        }
                    }
                });

                for (auto& img : imgBarriers) {
                    img.srcAccessMask = 0;
                    img.dstAccessMask = 0;
                }

                c[frameIndex].RecordT<CommandPipelineBarrier> (
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    std::vector<VkMemoryBarrier> {},
                    std::vector<VkBufferMemoryBarrier> {},
                    imgBarriers);

                Utils::ForEachP<ImageResource> (allInputs, [&] (const std::shared_ptr<ImageResource>& img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        layoutMap[imgbase] = op->GetImageLayoutAtEndForInputs (*img);
                    }
                });

                Utils::ForEachP<ImageResource> (allOutputs, [&] (const std::shared_ptr<ImageResource>& img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        layoutMap[imgbase] = op->GetImageLayoutAtEndForOutputs (*img);
                    }
                });
            }

            for (auto op : p.GetAllOperations ()) {
                op->Record (settings.connectionSet, frameIndex, c[frameIndex]);
            }

            for (auto op : p.GetAllOperations ()) {
                auto allInputs  = settings.connectionSet.GetPointingHere<Resource> (op);
                auto allOutputs = settings.connectionSet.GetPointingTo<Resource> (op);
                for (auto res : p.GetAllOutputs ()) {
                    res->OnPostWrite (frameIndex, c[frameIndex]);
                }
            }
        }

        std::vector<VkImageMemoryBarrier> transitionToInitial;
        for (Pass& p : passes) {
            Utils::ForEach<ImageResource*> (p.GetAllInputs (), [&] (ImageResource* img) {
                for (auto imgbase : img->GetImages (frameIndex)) {
                    const VkImageLayout currentLayout = layoutMap[imgbase];
                    if (currentLayout != img->GetInitialLayout ()) {
                        transitionToInitial.push_back (imgbase->GetBarrier (currentLayout, img->GetInitialLayout ()));
                    }
                }
            });
        }

        for (auto& img : transitionToInitial) {
            img.srcAccessMask = 0;
            img.dstAccessMask = 0;
        }

        c[frameIndex].RecordT<CommandPipelineBarrier> (
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            std::vector<VkMemoryBarrier> {},
            std::vector<VkBufferMemoryBarrier> {},
            transitionToInitial);

        for (Pass& p : passes) {
            for (auto res : p.GetAllInputs ()) {
                res->OnGraphExecutionEnded (frameIndex, c[frameIndex]);
            }
            for (auto res : p.GetAllOutputs ()) {
                res->OnGraphExecutionEnded (frameIndex, c[frameIndex]);
            }
        }

        c[frameIndex].End ();
    }

    graphSettings = std::move (settings);

    compiled = true;

    compileEvent ();
}


void RenderGraph::Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkSemaphore>& signalSemaphores, VkFence fenceToSignal)
{
    if (GVK_ERROR (!compiled)) {
        return;
    }

    if (GVK_ERROR (frameIndex >= framesInFlight)) {
        return;
    }


    // TODO
    std::vector<VkPipelineStageFlags> waitDstStageMasks (waitSemaphores.size (), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    CommandBuffer& cmd = c[frameIndex];

    device->GetGraphicsQueue ().Submit (waitSemaphores, waitDstStageMasks, { &cmd }, signalSemaphores, fenceToSignal);
}


void RenderGraph::Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores)
{
    GVK_ASSERT (swapchain.SupportsPresenting ());

    // TODO itt present queue kene
    swapchain.Present (device->GetGraphicsQueue (), imageIndex, waitSemaphores);
}

} // namespace RG

} // namespace GVK
