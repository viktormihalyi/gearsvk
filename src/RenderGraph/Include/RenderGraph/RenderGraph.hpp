#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP

#include "RenderGraph/RenderGraphAPI.hpp"

#include "Utils/Event.hpp"
#include "VulkanWrapper/Utils/VulkanUtils.hpp"
#include "VulkanWrapper/VulkanWrapper.hpp"
#include <memory>

#include "RenderGraph/GraphSettings.hpp"
#include "RenderGraph/RenderGraphPass.hpp"

#include <set>
#include <unordered_set>

namespace GVK {

namespace RG {

class Operation;
class Resource;
class GraphSettings;


class GVK_RENDERER_API RenderGraph final : public Noncopyable {
public:// TODO
    bool                       compiled;
    std::vector<Pass>          passes;
    std::vector<CommandBuffer> commandBuffers;
    std::vector<std::vector<CommandBuffer>> commandBuffers2;
    
    std::unordered_map<Image*, std::vector<VkImageLayout>> imageLayoutSequence;

public:
    GraphSettings graphSettings;

public:
    RenderGraph ();

    void Compile (GraphSettings&& settings);

    void Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkSemaphore>& signalSemaphores = {}, VkFence fence = VK_NULL_HANDLE);
    void Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores = {});

    uint32_t GetPassCount () const;

private:
    // utility functions for compilation
    void              CompileResources (const GraphSettings& settings);
    Pass              GetNextPass (const ConnectionSet& connectionSet, const Pass& lastPass) const;
    Pass              GetFirstPass (const ConnectionSet& connectionSet) const;
    std::vector<Pass> GetPasses (const ConnectionSet& connectionSet) const;
    void              SeparatePasses (const ConnectionSet& connectionSet);
};


} // namespace RG

} // namespace GVK

#endif