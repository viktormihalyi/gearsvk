#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP

#include "GearsVkAPI.hpp"

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

class CompileResultProvider {
public:
    USING_PTR_ABSTRACT (CompileResultProvider);

    virtual ~CompileResultProvider () = default;

    virtual std::vector<VkCommandBuffer> GetCommandBuffersToSubmit (uint32_t frameIndex)                          = 0;
    virtual CommandBuffer&               GetCommandBufferToRecord (uint32_t frameIndex, uint32_t operationIndex)  = 0;
    virtual std::vector<uint32_t>        GetOperationsToRecompile (uint32_t frameIndex, uint32_t operationIndex)  = 0;
    virtual uint32_t                     GetUsedCommandBuffer (uint32_t frameIndex, uint32_t operationIndex)      = 0;
    virtual std::vector<uint32_t>        GetOperationsToRecord (uint32_t frameIndex, uint32_t commandBufferIndex) = 0;
};

class OperationToCommandBufferMappingStrategy {
protected:
    const uint32_t operationCount;

public:
    USING_PTR_ABSTRACT (OperationToCommandBufferMappingStrategy);

    OperationToCommandBufferMappingStrategy (uint32_t operationCount)
        : operationCount (operationCount)
    {
    }

    virtual uint32_t              GetCommandBufferIndex (uint32_t operationIndex)   = 0;
    virtual std::vector<uint32_t> GetOperationIndices (uint32_t commandBufferIndex) = 0;
    virtual uint32_t              GetRequiredCommandBufferCount ()                  = 0;

    uint32_t GetOperationCount () const { return operationCount; }
};

// one line is one command buffer
// {f1, o1}, {f1, o2}, {f1, o3}, {f1, o4}, {f1, o5},
// {f2, o1}, {f2, o2}, {f2, o3}, {f2, o4}, {f2, o5},
// {f3, o1}, {f3, o2}, {f3, o3}, {f3, o4}, {f3, o5},

class DefaultOperationToCommandBufferMappingStrategy : public OperationToCommandBufferMappingStrategy {
public:
    USING_PTR (DefaultOperationToCommandBufferMappingStrategy);

    using OperationToCommandBufferMappingStrategy::OperationToCommandBufferMappingStrategy;

    virtual uint32_t GetCommandBufferIndex (uint32_t operationIndex) override
    {
        ASSERT (operationIndex < operationCount);
        return 0;
    }

    virtual std::vector<uint32_t> GetOperationIndices (uint32_t commandBufferIndex)
    {
        ASSERT (commandBufferIndex == 0);

        std::vector<uint32_t> result;
        for (uint32_t i = 0; i < operationCount; ++i) {
            result.push_back (i);
        }
        return result;
    }

    virtual uint32_t GetRequiredCommandBufferCount () override
    {
        return 1;
    }
}; // namespace RG

class SeperatedOperationToCommandBufferMappingStrategy : public OperationToCommandBufferMappingStrategy {
public:
    USING_PTR (SeperatedOperationToCommandBufferMappingStrategy);

    using OperationToCommandBufferMappingStrategy::OperationToCommandBufferMappingStrategy;

    virtual uint32_t GetCommandBufferIndex (uint32_t operationIndex) override
    {
        ASSERT (operationIndex < operationCount);
        return operationIndex;
    }

    virtual std::vector<uint32_t> GetOperationIndices (uint32_t commandBufferIndex)
    {
        return {commandBufferIndex};
    }

    virtual uint32_t GetRequiredCommandBufferCount () override
    {
        return operationCount;
    }
};


struct CommandBufferHolder {
public:
    std::vector<CommandBuffer::U> commandBuffers;
    std::vector<VkCommandBuffer>  commandBufferHandles;

    // one commandbuffer can hold multiple operations
    // one operation can only be inside one commandbuffer
    std::vector<uint32_t>              operationToCommandBufferMapping; // operation -> commandbuffer
    std::vector<std::vector<uint32_t>> commandBufferToOperationMapping; // commandbuffer -> operations

    CommandBufferHolder (VkDevice device, VkCommandPool commandPool, OperationToCommandBufferMappingStrategy& strategy)
    {
        const uint32_t commandBufferCount = strategy.GetRequiredCommandBufferCount ();
        for (uint32_t i = 0; i < commandBufferCount; ++i) {
            commandBuffers.push_back (CommandBuffer::Create (device, commandPool));
        }
        for (uint32_t i = 0; i < commandBufferCount; ++i) {
            commandBufferHandles.push_back (*commandBuffers[i]);
        }

        commandBufferToOperationMapping.resize (commandBufferCount);
        operationToCommandBufferMapping.resize (strategy.GetOperationCount ());

        for (uint32_t operationIndex = 0; operationIndex < strategy.GetOperationCount (); ++operationIndex) {
            operationToCommandBufferMapping[operationIndex] = strategy.GetCommandBufferIndex (operationIndex);
        }
        for (uint32_t commandBufferIndex = 0; commandBufferIndex < commandBufferCount; ++commandBufferIndex) {
            commandBufferToOperationMapping[commandBufferIndex] = strategy.GetOperationIndices (commandBufferIndex);
        }
    }


    uint32_t              GetCommandBufferIndices (uint32_t operationIndex) { return operationToCommandBufferMapping[operationIndex]; }
    std::vector<uint32_t> GetOperationIndices (uint32_t commandBufferIndex) { return commandBufferToOperationMapping[commandBufferIndex]; }
};

class GEARSVK_API RenderGraph final : public Noncopyable {
public:
    struct OutputConnection final {
        Operation& operation;
        uint32_t   binding;
        Resource&  resource;
    };

private:
    struct CompileResult : public CompileResultProvider {
        USING_PTR (CompileResult);

        std::vector<CommandBufferHolder> c; // one for each frame

        CompileResult (VkDevice device, VkCommandPool commandPool, uint32_t frameCount)
        {
            for (uint32_t i = 0; i < frameCount; ++i) {
                SeperatedOperationToCommandBufferMappingStrategy s (frameCount);
                c.emplace_back (device, commandPool, s);
            }
        }

        void BeginAll ()
        {
            for (auto& h : c)
                for (auto& cmd : h.commandBuffers)
                    cmd->Begin ();
        }

        void EndAll ()
        {
            for (auto& h : c)
                for (auto& cmd : h.commandBuffers)
                    cmd->End ();
        }

        void Clear ()
        {
            c.clear ();
        }

        virtual std::vector<uint32_t> GetOperationsToRecompile (uint32_t frameIndex, uint32_t operationIndex) override
        {
            const uint32_t commandBufferIndex = c[frameIndex].GetCommandBufferIndices (operationIndex);
            return c[frameIndex].GetOperationIndices (commandBufferIndex);
        }

        virtual uint32_t GetUsedCommandBuffer (uint32_t frameIndex, uint32_t operationIndex) override
        {
            const uint32_t commandBufferIndex = c[frameIndex].GetCommandBufferIndices (operationIndex);
            return c[frameIndex].GetCommandBufferIndices (commandBufferIndex);
        }

        virtual std::vector<uint32_t> GetOperationsToRecord (uint32_t frameIndex, uint32_t commandBufferIndex) override
        {
            return c[frameIndex].GetOperationIndices (commandBufferIndex);
        }

        virtual std::vector<VkCommandBuffer> GetCommandBuffersToSubmit (uint32_t frameIndex) override
        {
            return c[frameIndex].commandBufferHandles;
        }

        virtual CommandBuffer& GetCommandBufferToRecord (uint32_t frameIndex, uint32_t operationIndex) override
        {
            const uint32_t commandBufferIndex = c[frameIndex].GetCommandBufferIndices (operationIndex);

            return *c[frameIndex].commandBuffers[commandBufferIndex];
            //return *commandBuffers[frameIndex];
        }
    };

    bool compiled;

    const VkDevice      device;
    const VkCommandPool commandPool;

    GraphSettings            compileSettings;
    CompileResultProvider::U compileResult;

    std::vector<Resource::U>  resources;
    std::vector<Operation::U> operations;

public:
    USING_PTR (RenderGraph);

    RenderGraph (VkDevice device, VkCommandPool commandPool);

    Resource&  AddResource (Resource::U&& resource);
    Operation& AddOperation (Operation::U&& resource);


    template<typename ResourceType, typename... ARGS>
    ResourceType& CreateResource (ARGS&&... args)
    {
        compiled = false;

        resources.emplace_back (std::move (ResourceType::Create (std::forward<ARGS> (args)...)));
        return static_cast<ResourceType&> (*resources[resources.size () - 1]);
    }

    template<typename OperationType, typename... ARGS>
    OperationType& CreateOperation (ARGS&&... args)
    {
        compiled = false;

        operations.push_back (std::move (OperationType::Create (std::forward<ARGS> (args)...)));
        return static_cast<OperationType&> (*operations[operations.size () - 1]);
    }

    template<typename ConnectionType, typename ResourceType, typename... ARGS>
    void CreateInputConnection (Operation& op, uint32_t binding, ResourceType& res, ARGS&&... args)
    {
        compiled = false;

        op.inputs.push_back (res);
        op.newInputBindings.push_back (std::move (ConnectionType::Create (binding, res, std::forward<ARGS> (args)...)));
    }

    void CreateOutputConnection (Operation& operation, uint32_t binding, ImageResource& resource);

    void CompileResources (const GraphSettings& settings);
    void Compile (const GraphSettings& settings);
    void Recompile (uint32_t commandBufferIndex);
    void Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkSemaphore>& signalSemaphores = {}, VkFence fence = VK_NULL_HANDLE);

    void Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores = {})
    {
        ASSERT (swapchain.SupportsPresenting ());
        swapchain.Present (compileSettings.queue, imageIndex, waitSemaphores);
    }

    const GraphSettings& GetGraphSettings () const { return compileSettings; }
};


} // namespace RG

#endif