#include "RenderGraph.hpp"


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
    GVK_ASSERT (false);
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
                auto inputs  = op->GetPointingHere<Resource> ();
                auto outputs = op->GetPointingTo<Resource> ();

                for (auto res : inputs) {
                    res->OnPreRead (frameIndex, *c[frameIndex]);
                }

                for (auto res : outputs) {
                    res->OnPreWrite (frameIndex, *c[frameIndex]);
                }

                std::vector<VkImageMemoryBarrier> imgBarriers;

                Utils::ForEach<ImageResource*> (inputs, [&] (ImageResource* img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        const VkImageLayout currentLayout = layoutMap[imgbase];
                        const VkImageLayout newLayout     = op->GetImageLayoutAtStartForInputs (*img);
                        if (newLayout != currentLayout) {
                            imgBarriers.push_back (imgbase->GetBarrier (currentLayout, newLayout));
                            layoutMap[imgbase] = newLayout;
                        }
                    }
                });

                Utils::ForEach<ImageResource*> (outputs, [&] (ImageResource* img) {
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

                Utils::ForEach<ImageResource*> (inputs, [&] (ImageResource* img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        layoutMap[imgbase] = op->GetImageLayoutAtEndForInputs (*img);
                    }
                });

                Utils::ForEach<ImageResource*> (outputs, [&] (ImageResource* img) {
                    for (auto imgbase : img->GetImages (frameIndex)) {
                        layoutMap[imgbase] = op->GetImageLayoutAtEndForOutputs (*img);
                    }
                });
            }

            for (auto op : p.operations) {
                op->Record (frameIndex, *c[frameIndex]);
            }

            for (auto op : p.operations) {
                auto inputs  = op->GetPointingHere<Resource> ();
                auto outputs = op->GetPointingTo<Resource> ();
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


void RenderGraph::CreateInputConnection (RenderOperation& op, Resource& res, InputBindingU&& conn)
{
    compiled = false;

    res.AddConnectionTo (op);

    op.AddInput (std::move (conn));
}


void RenderGraph::CreateInputConnection (TransferOperation& op, ImageResource& res)
{
    compiled = false;

    res.AddConnectionTo (op);
}


void RenderGraph::CreateOutputConnection (RenderOperation& operation, uint32_t binding, ImageResource& resource)
{
    compiled = false;

    operation.AddConnectionTo (resource);

    operation.AddOutput (binding, resource);
}


void RenderGraph::CreateOutputConnection (TransferOperation& operation, Resource& resource)
{
    compiled = false;

    operation.AddConnectionTo (resource);
}


void RenderGraph::Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkSemaphore>& signalSemaphores, VkFence fenceToSignal)
{
    if (GVK_ERROR (!compiled)) {
        return;
    }

    if (GVK_ERROR (frameIndex >= compileSettings.framesInFlight)) {
        return;
    }


    // TODO
    std::vector<VkPipelineStageFlags> waitDstStageMasks (waitSemaphores.size (), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    CommandBufferU& cmd = c[frameIndex];

    compileSettings.GetDevice ().GetGraphicsQueue ().Submit (waitSemaphores, waitDstStageMasks, { cmd.get () }, signalSemaphores, fenceToSignal);
}


void RenderGraph::Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores)
{
    GVK_ASSERT (swapchain.SupportsPresenting ());

    // TODO itt present queue kene
    swapchain.Present (compileSettings.GetDevice ().GetGraphicsQueue (), imageIndex, waitSemaphores);
}

} // namespace RG
