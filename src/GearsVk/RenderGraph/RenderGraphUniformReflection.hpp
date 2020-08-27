#ifndef RENDERGRAPH_UNIFORM_REFLECTION_HPP
#define RENDERGRAPH_UNIFORM_REFLECTION_HPP

#include "Operation.hpp"
#include "RenderGraph.hpp"
#include "Resource.hpp"

#include "Assert.hpp"
#include "GearsVkAPI.hpp"
#include "UniformView.hpp"

#include <unordered_map>
#include <vector>

namespace RG {

class GEARSVK_API RenderGraphUniformReflection {
private:
    class UboSelector {
    private:
        std::unordered_map<std::string, SR::IUDataP> udatas;

    public:
        SR::IUData& operator[] (const std::string& uboName)
        {
            auto it = udatas.find (uboName);
            if (it != udatas.end ()) {
                return *it->second;
            }

            throw std::runtime_error ("no ubo named \"" + uboName + "\"");
        }

        void Set (const std::string& uboName, const SR::IUDataP& uboData)
        {
            udatas[uboName] = uboData;
        }
    };

    class ShaderKindSelector {
    private:
        std::unordered_map<ShaderModule::ShaderKind, UboSelector> uboSelectors;

    public:
        UboSelector& operator[] (ShaderModule::ShaderKind shaderKind)
        {
            auto it = uboSelectors.find (shaderKind);
            if (it != uboSelectors.end ()) {
                return it->second;
            }

            throw std::runtime_error ("no such shaderkind");
        }

        void Set (ShaderModule::ShaderKind shaderKind, UboSelector&& uboSel)
        {
            uboSelectors[shaderKind] = std::move (uboSel);
        }
    };

    struct CopyOperation {
        void*    destination;
        void*    source;
        uint64_t size;

        void Do () const
        {
            memcpy (destination, source, size);
        }
    };

    // operation uuid
    std::unordered_map<GearsVk::UUID, ShaderKindSelector> selectors;

    // copy between cpy memory (staging to vulkan cpu mapped)
    std::vector<std::vector<CopyOperation>> copyOperations;

    RG::RenderGraph&         graph;
    const RG::GraphSettings& settings;

    //
    std::vector<RG::InputBufferBindableResourceP>                                             uboResources;
    std::vector<std::tuple<RG::RenderOperationP, uint32_t, RG::InputBufferBindableResourceP>> uboConnections;
    std::unordered_map<GearsVk::UUID, SR::IUDataP>                                            udatas;

public:
    using Filter          = std::function<bool (const RG::RenderOperationP&, const ShaderModule&, const SR::UBOP&)>;
    using ResourceCreator = std::function<RG::InputBufferBindableResourceP (const RG::RenderOperationP&, const ShaderModule&, const SR::UBOP&)>;

public:
    static bool DefaultFilter (const RG::RenderOperationP&, const ShaderModule&, const SR::UBOP&)
    {
        return false;
    }

    static RG::InputBufferBindableResourceP DefaultResourceCreator (const RG::RenderOperationP&, const ShaderModule&, const SR::UBOP& ubo)
    {
        return RG::UniformBlockResource::CreateShared (*ubo);
    }

    static RG::InputBufferBindableResourceP GPUBufferResourceCreator (const RG::RenderOperationP&, const ShaderModule&, const SR::UBOP& ubo)
    {
        return RG::GPUBufferResource::CreateShared (ubo->GetFullSize ());
    }

public:
    RenderGraphUniformReflection (RG::RenderGraph&         graph,
                                  const RG::GraphSettings& settings,
                                  const Filter&            filter          = &DefaultFilter,
                                  const ResourceCreator&   resourceCreator = &DefaultResourceCreator);

public:
    // call after RG::RenderGraph::Compile (or RG::RenderGraph::CompileResources)
    void RecordCopyOperations ();

    void Flush (uint32_t frameIndex);

    ShaderKindSelector& operator[] (const RG::RenderOperation& renderOp)
    {
        return selectors.at (renderOp.GetUUID ());
    }

    ShaderKindSelector& operator[] (const RG::RenderOperationP& renderOp)
    {
        return (*this)[*renderOp];
    }

private:
    void CreateGraphResources (const Filter& filter, const ResourceCreator& resourceCreator);

    void CreateGraphConnections ();
};

} // namespace RG

#endif