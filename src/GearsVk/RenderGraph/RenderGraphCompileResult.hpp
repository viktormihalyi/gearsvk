#ifndef RGCR_HPP
#define RGCR_HPP

#include "CompileResultProvider.hpp"


USING_PTR (OperationToCommandBufferMappingStrategy);
class OperationToCommandBufferMappingStrategy {
protected:
    const uint32_t operationCount;

public:
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
USING_PTR (DefaultOperationToCommandBufferMappingStrategy);

class DefaultOperationToCommandBufferMappingStrategy : public OperationToCommandBufferMappingStrategy {
public:
    USING_CREATE (DefaultOperationToCommandBufferMappingStrategy);

    using OperationToCommandBufferMappingStrategy::OperationToCommandBufferMappingStrategy;

    virtual uint32_t GetCommandBufferIndex (uint32_t operationIndex) override
    {
        GVK_ASSERT (operationIndex < operationCount);
        return 0;
    }

    virtual std::vector<uint32_t> GetOperationIndices (uint32_t commandBufferIndex)
    {
        GVK_ASSERT (commandBufferIndex == 0);

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

USING_PTR (SeperatedOperationToCommandBufferMappingStrategy);
class SeperatedOperationToCommandBufferMappingStrategy : public OperationToCommandBufferMappingStrategy {
public:
    USING_CREATE (SeperatedOperationToCommandBufferMappingStrategy);

    using OperationToCommandBufferMappingStrategy::OperationToCommandBufferMappingStrategy;

    virtual uint32_t GetCommandBufferIndex (uint32_t operationIndex) override
    {
        GVK_ASSERT (operationIndex < operationCount);
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
    std::vector<CommandBufferU>  commandBuffers;
    std::vector<VkCommandBuffer> commandBufferHandles;

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


USING_PTR (CompileResult);
struct CompileResult : public CompileResultProvider {
    USING_CREATE (CompileResult);

    std::vector<CommandBufferHolder> c; // one for each frame

    CompileResult (const DeviceExtra& device, uint32_t frameCount)
    {
        for (uint32_t i = 0; i < frameCount; ++i) {
            DefaultOperationToCommandBufferMappingStrategy s (frameCount);
            //SeperatedOperationToCommandBufferMappingStrategy s (frameCount);
            c.emplace_back (device, device.GetCommandPool (), s);
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

#endif