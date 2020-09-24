#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP

#include "GearsVkAPI.hpp"

#include "CompileResultProvider.hpp"
#include "Event.hpp"
#include "Ptr.hpp"
#include "ShaderPipeline.hpp"
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


namespace RG {


USING_PTR (RenderGraph);
class GEARSVK_API RenderGraph final : public Noncopyable {
    USING_CREATE (RenderGraph);

private:
    bool                   compiled;
    GraphSettings          compileSettings;
    CompileResultProviderU compileResult;

    std::vector<ResourceP> resources;

public:
    std::vector<OperationP> operations;

    Event<> compileEvent;

public:
    RenderGraph ();


    // ------------------------- graph building functions -------------------------

    ResourceP AddResource (ResourceP resource);

    OperationP AddOperation (OperationP operation);

    template<typename ResourceType, typename... ARGS>
    P<ResourceType> CreateResource (ARGS&&... args);

    template<typename OperationType, typename... ARGS>
    P<OperationType> CreateOperation (ARGS&&... args);

    void CreateInputConnection (Operation& op, Resource& res, InputBindingU&& conn);
    void CreateOutputConnection (Operation& operation, uint32_t binding, ImageResource& resource);


    // ------------------------- compile / execute -------------------------

    void CompileResources (const GraphSettings& settings);
    void Compile (const GraphSettings& settings);
    void Recompile (uint32_t commandBufferIndex);

    void Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkSemaphore>& signalSemaphores = {}, VkFence fence = VK_NULL_HANDLE);
    void Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores = {});

    GraphSettings&       GetGraphSettings () { return compileSettings; }
    const GraphSettings& GetGraphSettings () const { return compileSettings; }

private:
    // utility functions for compilation
    std::set<Operation*>              GetNextOperations (const std::set<Operation*>& lastOperations) const;
    std::set<Operation*>              GetFirstPassOperations () const;
    std::vector<std::set<Operation*>> GetPasses () const;
};


template<typename ResourceType, typename... ARGS>
P<ResourceType> RenderGraph::CreateResource (ARGS&&... args)
{
    auto newRes = ResourceType::CreateShared (std::forward<ARGS> (args)...);
    AddResource (newRes);
    return newRes;
}


template<typename OperationType, typename... ARGS>
P<OperationType> RenderGraph::CreateOperation (ARGS&&... args)
{
    auto newOp = OperationType::CreateShared (std::forward<ARGS> (args)...);
    AddOperation (newOp);
    return newOp;
}


} // namespace RG

#endif