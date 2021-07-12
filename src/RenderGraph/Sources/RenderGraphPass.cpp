#include "RenderGraphPass.hpp"

#include "Utils/Assert.hpp"

#include <optional>

namespace GVK {

namespace RG {


std::set<Operation*> Pass::GetAllOperations () const
{
    std::set<Operation*> result;
    for (auto& op : operationIOs) {
        result.insert (op.op);
    }
    return result;
}


std::set<Resource*> Pass::GetAllInputs () const
{
    std::set<Resource*> result;
    for (auto& op : operationIOs) {
        for (Resource* inp : op.inputs) {
            result.insert (inp);
        }
    }
    return result;
}


std::set<Resource*> Pass::GetAllOutputs () const
{
    std::set<Resource*> result;
    for (auto& op : operationIOs) {
        for (Resource* inp : op.outputs) {
            result.insert (inp);
        }
    }
    return result;
}


void Pass::AddOutput (Operation* op, Resource* output)
{
    OperationIO* oIO = EnsureOperationIO (op);
    ResourceIO*  rIO = EnsureResourceIO (output);

    oIO->outputs.insert (output);
    rIO->writers.insert (op);
}


void Pass::AddInput (Operation* op, Resource* input)
{
    OperationIO* oIO = EnsureOperationIO (op);
    ResourceIO*  rIO = EnsureResourceIO (input);

    oIO->inputs.insert (input);
    rIO->readers.insert (op);
}


bool Pass::RemoveOutput (Operation* op, Resource* output)
{
    OperationIO* oIO = GetOperationIO (op);
    ResourceIO*  rIO = GetResourceIO (output);

    if (GVK_ERROR (oIO == nullptr || rIO == nullptr)) {
        return false;
    }

    if (GVK_ERROR (!oIO->outputs.count (output) || !rIO->writers.count (op))) {
        return false;
    }

    oIO->outputs.erase (output);
    rIO->writers.erase (op);

    RemoveIfEmpty (op);
    RemoveIfEmpty (output);

    return true;
}


bool Pass::RemoveInput (Operation* op, Resource* input)
{
    OperationIO* oIO = GetOperationIO (op);
    ResourceIO*  rIO = GetResourceIO (input);

    if (GVK_ERROR (oIO == nullptr || rIO == nullptr)) {
        return false;
    }

    if (GVK_ERROR (!oIO->inputs.count (input) || !rIO->readers.count (op))) {
        return false;
    }

    oIO->inputs.erase (input);
    rIO->readers.erase (op);

    RemoveIfEmpty (op);
    RemoveIfEmpty (input);

    return true;
}


Pass::ResourceIO* Pass::GetResourceIO (Resource* res)
{
    for (ResourceIO& rIO : resourceIOs) {
        if (rIO.res == res) {
            return &rIO;
        }
    }
    return nullptr;
}


Pass::OperationIO* Pass::GetOperationIO (Operation* op)
{
    for (OperationIO& oIO : operationIOs) {
        if (oIO.op == op) {
            return &oIO;
        }
    }
    return nullptr;
}


bool Pass::RemoveOperationIO (OperationIO* op)
{
    bool found = false;

    for (size_t i = 0; i < operationIOs.size (); ++i) {
        if (&operationIOs[i] == op) {
            found = true;
        }
    }

    if (GVK_ERROR (!found))
        return false;

    const std::set<Resource*> opOutputs = op->outputs;
    const std::set<Resource*> opInputs  = op->inputs;

    for (auto output : opOutputs)
        RemoveOutput (op->op, output);

    for (auto input : opInputs)
        RemoveInput (op->op, input);

    return true;
}


bool Pass::AddOperationIO (OperationIO* op)
{
    bool found = false;

    for (size_t i = 0; i < operationIOs.size (); ++i) {
        if (&operationIOs[i] == op) {
            found = true;
        }
    }

    if (GVK_ERROR (found))
        return false;

    for (auto output : op->outputs)
        AddOutput (op->op, output);

    for (auto input : op->inputs)
        AddInput (op->op, input);

    return true;
}



bool Pass::IsEmpty () const
{
    return operationIOs.empty () && resourceIOs.empty ();
}


bool Pass::RemoveIfEmpty (Operation* op)
{
    for (size_t i = 0; i < operationIOs.size (); ++i) {
        if (operationIOs[i].op == op && operationIOs[i].inputs.empty () && operationIOs[i].outputs.empty ()) {
            operationIOs.erase (operationIOs.begin () + i);
            return true;
        }
    }

    return false;
}


bool Pass::RemoveIfEmpty (Resource* res)
{
    for (size_t i = 0; i < resourceIOs.size (); ++i) {
        if (resourceIOs[i].res == res && resourceIOs[i].writers.empty () && resourceIOs[i].readers.empty ()) {
            resourceIOs.erase (resourceIOs.begin () + i);
            return true;
        }
    }

    return false;
}


Pass::ResourceIO* Pass::EnsureResourceIO (Resource* res)
{
    ResourceIO* existing = GetResourceIO (res);
    if (existing != nullptr) {
        return existing;
    }

    resourceIOs.emplace_back (ResourceIO { res, {}, {} });
    return &resourceIOs.back ();
}


Pass::OperationIO* Pass::EnsureOperationIO (Operation* op)
{
    OperationIO* existing = GetOperationIO (op);
    if (existing != nullptr) {
        return existing;
    }

    operationIOs.emplace_back (OperationIO { op, {}, {} });
    return &operationIOs.back ();
}


} // namespace RG

} // namespace GVK