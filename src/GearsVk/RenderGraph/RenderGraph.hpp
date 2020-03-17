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

class Graph final : public Noncopyable {
public:
    USING_PTR (Graph);

    VkDevice         device;
    VkCommandPool    commandPool;
    CommandBuffer::U cmdBuf;

    std::vector<Resource::U>  resources;
    std::vector<Operation::U> operations;

    uint32_t width;
    uint32_t height;

    Graph (VkDevice device, VkCommandPool commandPool, uint32_t width, uint32_t height);

    GraphInfo GetGraphInfo () const;

    Resource::Ref  CreateResource (Resource::U&& resource);
    Operation::Ref CreateOperation (Operation::U&& resource);

    void Compile ();
    void Submit (VkQueue queue);

}; // namespace RenderGraph

} // namespace RenderGraph

#endif