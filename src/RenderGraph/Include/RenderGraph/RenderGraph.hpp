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

namespace RG {

class Operation;
class Resource;
class GraphSettings;


class GVK_RENDERER_API RenderGraph final : public Noncopyable {
public:// TODO
    bool                       compiled;
    std::vector<Pass>          passes;
    std::vector<GVK::CommandBuffer> commandBuffers;
    
    std::unordered_map<VkImage, std::vector<VkImageLayout>> imageLayoutSequence;

public:
    GraphSettings graphSettings;

public:
    RenderGraph ();

    void Compile (GraphSettings&& settings);

    void Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkSemaphore>& signalSemaphores = {}, VkFence fence = VK_NULL_HANDLE);
    void Present (uint32_t imageIndex, GVK::Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores = {});

    uint32_t GetPassCount () const;

private:
    void CompileResources ();
    void CompileOperations ();
    Pass GetNextPass (const Pass& lastPass) const;
    Pass GetFirstPass () const;
    void CreatePasses ();
    void SeparatePasses ();
    void DebugPrint ();
};


} // namespace RG

#endif