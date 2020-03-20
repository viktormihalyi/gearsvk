#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP

#include "Shaderpipeline.hpp"
#include "Timer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "GraphInfo.hpp"
#include "Operation.hpp"
#include "Resource.hpp"

namespace RenderGraph {

struct GraphConnection final {
    enum class Type {
        Input,
        Output
    };
    Type       type;
    Operation& operation;
    uint32_t   binding;
    Resource&  resource;
};


class Graph final : public Noncopyable {
public:
    USING_PTR (Graph);

    VkDevice      device;
    VkCommandPool commandPool;

    std::vector<CommandBuffer::U> commandBuffers;

    std::vector<Resource::U>  resources;
    std::vector<Operation::U> operations;

    uint32_t framesInFlight;
    uint32_t width;
    uint32_t height;

    Graph (VkDevice device, VkCommandPool commandPool, uint32_t framesInFlight, uint32_t width, uint32_t height);

    GraphInfo GetGraphInfo () const;

    Resource&  CreateResource (Resource::U&& resource);
    Operation& CreateOperation (Operation::U&& resource);

    void AddConnections (const std::vector<GraphConnection>& connections)
    {
        for (const GraphConnection& c : connections) {
            if (c.type == GraphConnection::Type::Input) {
                c.operation.AddInput (c.binding, c.resource);
            } else {
                c.operation.AddOutput (c.binding, c.resource);
            }
        }
    }

    void Compile ();
    void Submit (VkQueue queue, uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkSemaphore>& signalSemaphores = {});

}; // namespace RenderGraph

} // namespace RenderGraph

#endif