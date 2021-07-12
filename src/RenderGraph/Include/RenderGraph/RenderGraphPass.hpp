#ifndef RG_RENDERGRAPHPASS_HPP
#define RG_RENDERGRAPHPASS_HPP

#include "RenderGraph/RenderGraphAPI.hpp"

#include <vector>
#include <set>

namespace GVK {

namespace RG {

class Operation;
class Resource;

class GVK_RENDERER_API Pass {
public:
    struct OperationIO {
        Operation*          op;
        std::set<Resource*> inputs;
        std::set<Resource*> outputs;
    };

    struct ResourceIO {
        Resource*            res;
        std::set<Operation*> writers;
        std::set<Operation*> readers;
    };

private:
    std::vector<OperationIO> operationIOs;
    std::vector<ResourceIO>  resourceIOs;

public:
    std::set<Operation*> GetAllOperations () const;
    std::set<Resource*>  GetAllInputs () const;
    std::set<Resource*>  GetAllOutputs () const;

    void AddOutput (Operation* op, Resource* output);
    void AddInput (Operation* op, Resource* input);

    bool RemoveOutput (Operation* op, Resource* output);
    bool RemoveInput (Operation* op, Resource* input);

    ResourceIO*  GetResourceIO (Resource* res);
    OperationIO* GetOperationIO (Operation* op);

    bool RemoveOperationIO (OperationIO*);
    bool AddOperationIO (OperationIO*);

    bool IsEmpty () const;

private:
    bool RemoveIfEmpty (Operation* op);
    bool RemoveIfEmpty (Resource* res);

    ResourceIO* EnsureResourceIO (Resource* res);
    OperationIO* EnsureOperationIO (Operation* op);
};

} // namespace RG

} // namespace GVK

#endif