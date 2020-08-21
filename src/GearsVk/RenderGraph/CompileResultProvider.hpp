#ifndef COMPILERESULTPROVIDER_HPP
#define COMPILERESULTPROVIDER_HPP

#include "CommandBuffer.hpp"
#include "Ptr.hpp"

#include <cstdlib>
#include <vector>

USING_PTR (CompileResultProvider);
class CompileResultProvider {
public:
    virtual ~CompileResultProvider () = default;

    virtual std::vector<VkCommandBuffer> GetCommandBuffersToSubmit (uint32_t frameIndex)                          = 0;
    virtual CommandBuffer&               GetCommandBufferToRecord (uint32_t frameIndex, uint32_t operationIndex)  = 0;
    virtual std::vector<uint32_t>        GetOperationsToRecompile (uint32_t frameIndex, uint32_t operationIndex)  = 0;
    virtual uint32_t                     GetUsedCommandBuffer (uint32_t frameIndex, uint32_t operationIndex)      = 0;
    virtual std::vector<uint32_t>        GetOperationsToRecord (uint32_t frameIndex, uint32_t commandBufferIndex) = 0;
};

#endif