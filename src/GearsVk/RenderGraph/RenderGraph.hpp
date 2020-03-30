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

namespace RenderGraph {


class Graph final : public Noncopyable {
public:
    struct InputConnection final {
        Operation& operation;
        uint32_t   binding;
        Resource&  resource;

        bool operator< (const InputConnection& other) { return binding < other.binding; }
    };

    struct OutputConnection final {
        Operation& operation;
        uint32_t   binding;
        Resource&  resource;

        bool operator< (const OutputConnection& other) { return binding < other.binding; }
    };

private:
    bool compiled;

    const VkDevice      device;
    const VkCommandPool commandPool;

    std::vector<CommandBuffer::U> commandBuffers;

    std::vector<Resource::U>  resources;
    std::vector<Operation::U> operations;

    const GraphSettings settings;

public:
    USING_PTR (Graph);

    Graph (VkDevice device, VkCommandPool commandPool, GraphSettings settings);

    Resource&  CreateResource (Resource::U&& resource);
    Operation& CreateOperation (Operation::U&& resource);


    template<typename ResourceType, typename... ARGS>
    ResourceType& CreateResourceTyped (ARGS&&... args)
    {
        compiled = false;

        resources.push_back (std::move (ResourceType::Create (settings, std::forward<ARGS> (args)...)));
        return static_cast<ResourceType&> (*resources[resources.size () - 1]);
    }

    template<typename OperationType, typename... ARGS>
    OperationType& CreateOperationTyped (ARGS&&... args)
    {
        compiled = false;

        operations.push_back (std::move (OperationType::Create (settings, std::forward<ARGS> (args)...)));
        return static_cast<OperationType&> (*operations[operations.size () - 1]);
    }

    void AddConnection (const InputConnection& c);
    void AddConnection (const OutputConnection& c);

    void Compile ();
    void Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkSemaphore>& signalSemaphores = {}, VkFence fence = VK_NULL_HANDLE);

    void Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores = {})
    {
        ASSERT (swapchain.SupportsPresenting ());
        swapchain.Present (settings.queue, imageIndex, waitSemaphores);
    }

    const GraphSettings& GetGraphSettings () const { return settings; }
};


} // namespace RenderGraph

#endif