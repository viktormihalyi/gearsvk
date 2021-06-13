#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP

#include "GearsVk/GearsVkAPI.hpp"

#include "Utils/Event.hpp"
#include "GearsVk/VulkanWrapper/Utils/VulkanUtils.hpp"
#include "GearsVk/VulkanWrapper/VulkanWrapper.hpp"
#include <memory>

#include "GearsVk/RenderGraph/GraphSettings.hpp"

#include <set>
#include <unordered_set>

namespace GVK {

namespace RG {

class Operation;
class Resource;
class GraphSettings;


class GVK_RENDERER_API RenderGraph final : public Noncopyable {
private:
    bool compiled;

public:
    GraphSettings graphSettings;

    const DeviceExtra* device;
    uint32_t           framesInFlight;

public:
    class Pass {
    public:
        struct OperationIO {
            Operation*          op;
            std::set<Resource*> inputs;
            std::set<Resource*> outputs;
        };
        
        struct ResourceIO {
            Resource* res;
            std::set<Operation*> writers;
            std::set<Operation*> readers;
        };

    private:
        std::vector<OperationIO> operationIOs;
        std::vector<ResourceIO> resourceIOs;

    public:
        std::set<Operation*> GetAllOperations () const
        {
            std::set<Operation*> result;
            for (auto& op : operationIOs) {
                result.insert (op.op);
            }
            return result;
        }

        std::set<Resource*> GetAllInputs () const
        {
            std::set<Resource*> result;
            for (auto& op : operationIOs) {
                for (Resource* inp : op.inputs) {
                    result.insert (inp);
                }
            }
            return result;
        }

        std::set<Resource*> GetAllOutputs () const
        {
            std::set<Resource*> result;
            for (auto& op : operationIOs) {
                for (Resource* inp : op.outputs) {
                    result.insert (inp);
                }
            }
            return result;
        }

        void AddOutput (Operation* op, Resource* output)
        {
            OperationIO* oIO = EnsureOperationIO (op);
            ResourceIO* rIO = EnsureResourceIO (output);

            oIO->outputs.insert (output);
            rIO->writers.insert (op);
        }
        
        void AddInput (Operation* op, Resource* input)
        {
            OperationIO* oIO = EnsureOperationIO (op);
            ResourceIO*  rIO = EnsureResourceIO (input);

            oIO->inputs.insert (input);
            rIO->readers.insert (op);
        }

        bool RemoveOutput (Operation* op, Resource* output)
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

        bool RemoveInput (Operation* op, Resource* input)
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

        ResourceIO* GetResourceIO (Resource* res)
        {
            for (ResourceIO& rIO : resourceIOs) {
                if (rIO.res == res) {
                    return &rIO;
                }
            }
            return nullptr;
        }

        OperationIO* GetOperationIO (Operation* op)
        {
            for (OperationIO& oIO : operationIOs) {
                if (oIO.op == op) {
                    return &oIO;
                }
            }
            return nullptr;
        }

    private:
        bool RemoveIfEmpty (Operation* op)
        {
            for (size_t i = 0; i < operationIOs.size (); ++i) {
                if (operationIOs[i].op == op && operationIOs[i].inputs.empty () && operationIOs[i].outputs.empty ()) {
                    operationIOs.erase (operationIOs.begin () + i);
                    return true;
                }
            }

            return false;
        }

        bool RemoveIfEmpty (Resource* res)
        {
            for (size_t i = 0; i < resourceIOs.size (); ++i) {
                if (resourceIOs[i].res == res && resourceIOs[i].writers.empty () && resourceIOs[i].readers.empty ()) {
                    resourceIOs.erase (resourceIOs.begin () + i);
                    return true;
                }
            }

            return false;
        }

        ResourceIO* EnsureResourceIO (Resource* res)
        {
            ResourceIO* existing = GetResourceIO (res);
            if (existing != nullptr) {
                return existing;
            }

            resourceIOs.emplace_back (ResourceIO { res, {}, {} });
            return &resourceIOs.back ();
        }

        OperationIO* EnsureOperationIO (Operation* op)
        {
            OperationIO* existing = GetOperationIO (op);
            if (existing != nullptr) {
                return existing;
            }

            operationIOs.emplace_back (OperationIO { op, {}, {} });
            return &operationIOs.back ();
        }


    };

private:
    std::vector<Pass> passes;

    std::vector<CommandBuffer> c;

public:
    Event<> compileEvent;

public:
    RenderGraph ();

    void CompileResources (const GraphSettings& settings);
    void Compile (GraphSettings&& settings);

    void Submit (uint32_t frameIndex, const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkSemaphore>& signalSemaphores = {}, VkFence fence = VK_NULL_HANDLE);
    void Present (uint32_t imageIndex, Swapchain& swapchain, const std::vector<VkSemaphore>& waitSemaphores = {});

private:
    // utility functions for compilation
    Pass              GetNextPass (const ConnectionSet& connectionSet, const Pass& lastPass) const;
    Pass              GetFirstPass (const ConnectionSet& connectionSet) const;
    std::vector<Pass> GetPasses (const ConnectionSet& connectionSet) const;
    void              SeparatePasses (const ConnectionSet& connectionSet);
};


} // namespace RG

} // namespace GVK

#endif