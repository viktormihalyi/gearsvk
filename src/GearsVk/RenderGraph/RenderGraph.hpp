#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP

#include "GearsVkAPI.hpp"

#include "Event.hpp"
#include "Ptr.hpp"
#include "ShaderPipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "GraphSettings.hpp"
#include "Operation.hpp"
#include "Resource.hpp"

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <utility>


namespace RG {


USING_PTR (RenderGraph);
class GEARSVK_API RenderGraph final : public Noncopyable {
    USING_CREATE (RenderGraph);

private:
    bool compiled;

public:
    const DeviceExtra* device;
    uint32_t           framesInFlight;

private:
    struct Pass {
        std::set<Operation*> operations;
        std::set<Resource*>  inputs;
        std::set<Resource*>  outputs;
    };

    std::vector<Pass> passes;

    std::vector<CommandBufferU> c;

public:
    Event<> compileEvent;

public:
    RenderGraph ();

    void CompileResources (const GraphSettings& settings);
    void Compile (const GraphSettings& settings);
    void Recompile (uint32_t commandBufferIndex);

    void Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkSemaphore>& signalSemaphores = {}, VkFence fence = VK_NULL_HANDLE);
    void Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores = {});

private:
    // utility functions for compilation
    Pass              GetNextPass (const ConnectionSet& connectionSet, const Pass& lastPass) const;
    Pass              GetFirstPass (const ConnectionSet& connectionSet) const;
    std::vector<Pass> GetPasses (const ConnectionSet& connectionSet) const;
};


} // namespace RG

#endif