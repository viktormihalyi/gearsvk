#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP

#include "Shaderpipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "GraphSettings.hpp"
#include "Operation.hpp"
#include "Resource.hpp"

namespace RenderGraph {

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


class Graph final : public Noncopyable {
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

    void AddConnection (InputConnection& c);
    void AddConnection (OutputConnection& c);

    void Compile ();
    void Submit (VkQueue queue, uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkSemaphore>& signalSemaphores = {});

    GraphSettings GetGraphSettings () const { return settings; }
};


} // namespace RenderGraph

#endif