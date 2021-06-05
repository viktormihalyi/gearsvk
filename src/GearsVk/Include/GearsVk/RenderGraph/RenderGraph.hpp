#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP

#include "GearsVkAPI.hpp"

#include "Utils/Event.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"
#include <memory>

#include "GraphSettings.hpp"

#include <set>

namespace GVK {

namespace RG {

class Operation;
class Resource;
class GraphSettings;


class GVK_RENDERER_API RenderGraph final : public Noncopyable {
private:
    bool compiled;

public:
    GraphSettings graphSettings;

    const DeviceExtra* device;
    uint32_t           framesInFlight;

private:
    struct Pass {
        std::set<Operation*> operations;
        std::set<Resource*>  inputs;
        std::set<Resource*>  outputs;
    };

    std::vector<Pass> passes;

    std::vector<CommandBuffer> c;

public:
    Event<> compileEvent;

public:
    RenderGraph ();

    void CompileResources (const GraphSettings& settings);
    void Compile (GraphSettings&& settings);

    void Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkSemaphore>& signalSemaphores = {}, VkFence fence = VK_NULL_HANDLE);
    void Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores = {});

private:
    // utility functions for compilation
    Pass              GetNextPass (const ConnectionSet& connectionSet, const Pass& lastPass) const;
    Pass              GetFirstPass (const ConnectionSet& connectionSet) const;
    std::vector<Pass> GetPasses (const ConnectionSet& connectionSet) const;
    void              SeparatePasses (const ConnectionSet& connectionSet);
};


} // namespace RG

} // namespace GVK

#endif