#include "RenderGraph.hpp"

#include "GraphSettings.hpp"
#include "Operation.hpp"
#include "Resource.hpp"


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
    Utils::ForEachP<Resource> (settings.connectionSet.nodes, [&] (ResourceP& res) {
        res->Compile (settings);
    });
}


RenderGraph::Pass RenderGraph::GetNextPass (const ConnectionSet& connectionSet, const Pass& lastPass) const
{
    RenderGraph::Pass result;

    for (Operation* op : lastPass.operations) {
        for (const ResourceP& res : connectionSet.GetPointingTo<Resource> (op)) {
            for (const OperationP& nextOp : connectionSet.GetPointingTo<Operation> (res.get ())) {
                result.operations.insert (nextOp.get ());
            }
        }
    }

    for (auto& op : result.operations) {
        std::vector<ResourceP> inputs  = connectionSet.GetPointingHere<Resource> (op);
        std::vector<ResourceP> outputs = connectionSet.GetPointingTo<Resource> (op);
        for (ResourceP input : inputs) {
            result.inputs.insert (input.get ());
        }
        for (ResourceP output : outputs) {
            result.outputs.insert (output.get ());
        }
    }

    return result;
}


RenderGraph::Pass RenderGraph::GetFirstPass (const ConnectionSet& connectionSet) const
{
    std::set<Operation*> result;

    Utils::ForEachP<Operation> (connectionSet.nodes, [&] (const OperationP& op) {
        const std::vector<ResourceP> opInputs = connectionSet.GetPointingHere<Resource> (op.get ());

        const bool allInputsAreFirstWrittenByThisOp = std::all_of (opInputs.begin (), opInputs.end (), [&] (const ResourceP& res) {
            return connectionSet.GetPointingHere<Node> (res.get ()).empty ();
        });

        if (allInputsAreFirstWrittenByThisOp || opInputs.empty ()) {
            result.insert (op.get ());
        }
    });

    RenderGraph::Pass actualResult;
    actualResult.operations = result;

    for (auto& op : actualResult.operations) {
        std::vector<ResourceP> inputs  = connectionSet.GetPointingHere<Resource> (op);
        std::vector<ResourceP> outputs = connectionSet.GetPointingTo<Resource> (op);
        for (ResourceP input : inputs) {
            actualResult.inputs.insert (input.get ());
        }
        for (ResourceP output : outputs) {
            actualResult.outputs.insert (output.get ());
        }
    }

    return actualResult;
}


std::vector<RenderGraph::Pass> RenderGraph::GetPasses (const ConnectionSet& connectionSet) const
{
    std::vector<Pass> result;

    Pass nextPass = GetFirstPass (connectionSet);
    do {
        result.push_back (nextPass);
        nextPass = GetNextPass (connectionSet, nextPass);
    } while (!nextPass.operations.empty ());

    return result;
}


void RenderGraph::Compile (const GraphSettings& settings)
{
    device         = settings.device;
    framesInFlight = settings.framesInFlight;

    settings.GetDevice ().Wait ();
    vkQueueWaitIdle (settings.GetDevice ().GetGraphicsQueue ());

    passes = GetPasses (settings.connectionSet);

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

    std::unordered_map<ImageBase*, VkImageLayout> layoutMap;

    for (Pass& p : passes) {
        Utils::ForEach<ImageResource*> (p.inputs, [&] (ImageResource* img) {
            for (auto i : img->GetImages ()) {
                layoutMap[i] = img->GetInitialLayout ();
            }
        });
        Utils::ForEach<ImageResource*> (p.outputs, [&] (ImageResource* img) {
            for (auto i : img->GetImages ()) {
                layoutMap[i] = img->GetInitialLayout ();
            }
        });
    }

    c.clear ();
    const std::unordered_map<ImageBase*, VkImageLayout> layoutMapStart;
    for (uint32_t frameIndex = 0; frameIndex < settings.framesInFlight; ++frameIndex) {
        c.push_back (CommandBuffer::Create (settings.GetDevice ()));

        c[frameIndex]->Begin ();

        for (Pass& p : passes) {
            for (auto res : p.inputs) {
                res->OnGraphExecutionStarted (frameIndex, *c[frameIndex]);
            }
            for (auto res : p.outputs) {
                res->OnGraphExecutionStarted (frameIndex, *c[frameIndex]);
            }
        }

        for (Pass& p : passes) {
            for (auto op : p.operations) {
                auto inputs  = settings.connectionSet.GetPointingHere<Resource> (op);
                auto outputs = settings.connectionSet.GetPointingTo<Resource> (op);

                for (auto res : inputs) {
                    res->OnPreRead (frameIndex, *c[frameIndex]);
                }

                for (auto res : outputs) {
                    res->OnPreWrite (frameIndex, *c[frameIndex]);
                }

                std::vector<VkImageMemoryBarrier> imgBarriers;

                Utils::ForEachP<ImageResource> (inputs, [&] (const ImageResourceP& img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        const VkImageLayout currentLayout = layoutMap[imgbase];
                        const VkImageLayout newLayout     = op->GetImageLayoutAtStartForInputs (*img);
                        if (newLayout != currentLayout) {
                            imgBarriers.push_back (imgbase->GetBarrier (currentLayout, newLayout));
                            layoutMap[imgbase] = newLayout;
                        }
                    }
                });

                Utils::ForEachP<ImageResource> (outputs, [&] (const ImageResourceP& img) {
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

                c[frameIndex]->RecordT<CommandPipelineBarrier> (
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    std::vector<VkMemoryBarrier> {},
                    std::vector<VkBufferMemoryBarrier> {},
                    imgBarriers);

                Utils::ForEachP<ImageResource> (inputs, [&] (const ImageResourceP& img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        layoutMap[imgbase] = op->GetImageLayoutAtEndForInputs (*img);
                    }
                });

                Utils::ForEachP<ImageResource> (outputs, [&] (const ImageResourceP& img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        layoutMap[imgbase] = op->GetImageLayoutAtEndForOutputs (*img);
                    }
                });
            }

            for (auto op : p.operations) {
                op->Record (settings.connectionSet, frameIndex, *c[frameIndex]);
            }

            for (auto op : p.operations) {
                auto inputs  = settings.connectionSet.GetPointingHere<Resource> (op);
                auto outputs = settings.connectionSet.GetPointingTo<Resource> (op);
                for (auto res : p.outputs) {
                    res->OnPostWrite (frameIndex, *c[frameIndex]);
                }
            }
        }

        std::vector<VkImageMemoryBarrier> transitionToInitial;
        for (Pass& p : passes) {
            Utils::ForEach<ImageResource*> (p.inputs, [&] (ImageResource* img) {
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

        c[frameIndex]->RecordT<CommandPipelineBarrier> (
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            std::vector<VkMemoryBarrier> {},
            std::vector<VkBufferMemoryBarrier> {},
            transitionToInitial);

        for (Pass& p : passes) {
            for (auto res : p.inputs) {
                res->OnGraphExecutionEnded (frameIndex, *c[frameIndex]);
            }
            for (auto res : p.outputs) {
                res->OnGraphExecutionEnded (frameIndex, *c[frameIndex]);
            }
        }

        c[frameIndex]->End ();
    }

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

    CommandBufferU& cmd = c[frameIndex];

    device->GetGraphicsQueue ().Submit (waitSemaphores, waitDstStageMasks, { cmd.get () }, signalSemaphores, fenceToSignal);
}


void RenderGraph::Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores)
{
    GVK_ASSERT (swapchain.SupportsPresenting ());

    // TODO itt present queue kene
    swapchain.Present (device->GetGraphicsQueue (), imageIndex, waitSemaphores);
}

} // namespace RG
