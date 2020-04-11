#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP

#include "Shaderpipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "Connections.hpp"
#include "GraphSettings.hpp"
#include "Operation.hpp"
#include "Resource.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>

namespace RenderGraphns {


class RenderGraph final : public Noncopyable {
public:
    struct InputConnection final {
        Operation& operation;
        uint32_t   binding;
        Resource&  resource;
    };

    struct OutputConnection final {
        Operation& operation;
        uint32_t   binding;
        Resource&  resource;
    };

private:
    struct CompileResult {
        std::vector<CommandBuffer::U> commandBuffers;

        void Clear ()
        {
            commandBuffers.clear ();
        }
    };

    bool compiled;

    const VkDevice      device;
    const VkCommandPool commandPool;

    GraphSettings compileSettings;
    CompileResult compileResult;

    std::vector<Resource::U>  resources;
    std::vector<Operation::U> operations;


public:
    USING_PTR (RenderGraph);

    RenderGraph (VkDevice device, VkCommandPool commandPool);

    Resource&  AddResource (Resource::U&& resource);
    Operation& AddOperation (Operation::U&& resource);


    template<typename ResourceType, typename... ARGS>
    ResourceType& CreateResourceTyped (ARGS&&... args)
    {
        compiled = false;

        resources.push_back (std::move (ResourceType::Create (std::forward<ARGS> (args)...)));
        return static_cast<ResourceType&> (*resources[resources.size () - 1]);
    }

    template<typename OperationType, typename... ARGS>
    OperationType& CreateOperationTyped (ARGS&&... args)
    {
        compiled = false;

        operations.push_back (std::move (OperationType::Create (std::forward<ARGS> (args)...)));
        return static_cast<OperationType&> (*operations[operations.size () - 1]);
    }

    void AddConnection (const InputConnection& c);
    void AddConnection (const OutputConnection& c);

    void CompileResources (const GraphSettings& settings);
    void Compile (const GraphSettings& settings);
    void Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkSemaphore>& signalSemaphores = {}, VkFence fence = VK_NULL_HANDLE);

    void Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores = {})
    {
        ASSERT (swapchain.SupportsPresenting ());
        swapchain.Present (compileSettings.queue, imageIndex, waitSemaphores);
    }

    const GraphSettings& GetGraphSettings () const { return compileSettings; }
};


} // namespace RenderGraphns

#endif