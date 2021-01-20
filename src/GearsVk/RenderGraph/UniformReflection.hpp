#ifndef RENDERGRAPH_UNIFORM_REFLECTION_HPP
#define RENDERGRAPH_UNIFORM_REFLECTION_HPP

#include "Operation.hpp"
#include "RenderGraph.hpp"
#include "Resource.hpp"

#include "Assert.hpp"
#include "GearsVkAPI.hpp"
#include "UniformView.hpp"
#include "glmlib.hpp"

#include <tuple>
#include <unordered_map>
#include <vector>

namespace RG {

// TODO add possibility to store uniforms in gpu memory

// adds resources to and existing rendergraph based on the renderoperations inside it
// modifying uniforms takes place in a staging cpu memory, calling Flush will copy these to the actual uniform memory
// accessing a uniforms is available with operator[] eg.: reflection[Ptr<RG::RenderOperation>][ShaderKind][std::string][std::string]...

class GEARSVK_API ImageMap {
private:
    std::vector<std::pair<SR::Sampler, Ptr<ReadOnlyImageResource>>> images;

public:
    ImageMap ();

    Ptr<ReadOnlyImageResource> FindByName (const std::string& name) const;

    void Put (const SR::Sampler& sampler, const Ptr<ReadOnlyImageResource>& res);
};

using CreateParams                 = std::tuple<glm::uvec3, VkFormat, VkFilter>;
using ExtentProviderForImageCreate = std::function<std::optional<CreateParams> (const SR::Sampler& sampler)>;

GEARSVK_API
ImageMap CreateEmptyImageResources (RG::ConnectionSet& connectionSet);

GEARSVK_API
ImageMap CreateEmptyImageResources (RG::ConnectionSet& connectionSet, const ExtentProviderForImageCreate& extentProvider);


USING_PTR (UniformReflection);
class GEARSVK_API UniformReflection final : public EventObserver {
    USING_CREATE (UniformReflection);

private:
    class GEARSVK_API UboSelector {
    private:
        std::unordered_map<std::string, Ptr<SR::IUData>> udatas;

    public:
        SR::IUData& operator[] (const std::string& uboName)
        {
            auto it = udatas.find (uboName);
            if (it != udatas.end ()) {
                return *it->second;
            }

            GVK_ASSERT (false);
            return SR::dummyUData;
        }

        void Set (const std::string& uboName, const Ptr<SR::IUData>& uboData)
        {
            udatas[uboName] = uboData;
        }

        friend class UniformReflection;
    };

    class GEARSVK_API ShaderKindSelector {
    private:
        std::unordered_map<ShaderKind, UboSelector> uboSelectors;

    public:
        UboSelector& operator[] (ShaderKind shaderKind)
        {
            auto it = uboSelectors.find (shaderKind);
            if (it != uboSelectors.end ()) {
                return it->second;
            }

            GVK_ASSERT (false);
            throw std::runtime_error ("no such shaderkind");
        }

        void Set (ShaderKind shaderKind, UboSelector&& uboSel)
        {
            uboSelectors[shaderKind] = std::move (uboSel);
        }

        friend class UniformReflection;
    };

public:
    using ShaderUniforms   = UboSelector;
    using PipelineUniforms = ShaderKindSelector;

public:
    // operation uuid
    std::unordered_map<GearsVk::UUID, ShaderKindSelector> selectors;

    RG::ConnectionSet& connectionSet;

    //
    std::vector<Ptr<RG::InputBufferBindableResource>>                                                           uboResources;
    std::vector<std::tuple<Ptr<RG::RenderOperation>, uint32_t, Ptr<RG::InputBufferBindableResource>, ShaderKind>> uboConnections;
    std::unordered_map<GearsVk::UUID, Ptr<SR::IUData>>                                                          udatas;

public:
    using Filter          = std::function<bool (const Ptr<RG::RenderOperation>&, const ShaderModule&, const Ptr<SR::UBO>&)>;
    using ResourceCreator = std::function<Ptr<RG::InputBufferBindableResource> (const Ptr<RG::RenderOperation>&, const ShaderModule&, const Ptr<SR::UBO>&)>;

public:
    static bool DefaultFilter (const Ptr<RG::RenderOperation>&, const ShaderModule&, const Ptr<SR::UBO>&)
    {
        return false;
    }

    static Ptr<RG::InputBufferBindableResource> DefaultResourceCreator (const Ptr<RG::RenderOperation>&, const ShaderModule&, const Ptr<SR::UBO>& ubo)
    {
        return RG::CPUBufferResource::Create (ubo->GetFullSize ());
    }

    static Ptr<RG::InputBufferBindableResource> GPUBufferResourceCreator (const Ptr<RG::RenderOperation>&, const ShaderModule&, const Ptr<SR::UBO>& ubo)
    {
        return RG::GPUBufferResource::Create (ubo->GetFullSize ());
    }

public:
    UniformReflection (RG::ConnectionSet&     connectionSet,
                       const Filter&          filter          = &DefaultFilter,
                       const ResourceCreator& resourceCreator = &DefaultResourceCreator);

    void Flush (uint32_t frameIndex);

    ShaderKindSelector& operator[] (const RG::RenderOperation& renderOp);
    ShaderKindSelector& operator[] (const Ptr<RG::RenderOperation>& renderOp);
    ShaderKindSelector& operator[] (const GearsVk::UUID& renderOpId);

private:
    void CreateGraphResources (const Filter& filter, const ResourceCreator& resourceCreator);

    void CreateGraphConnections ();

public:
    void PrintDebugInfo ();
};


inline UniformReflection::ShaderKindSelector& UniformReflection::operator[] (const GearsVk::UUID& renderOpId)
{
    GVK_ASSERT (selectors.find (renderOpId) != selectors.end ());
    return selectors.at (renderOpId);
}


inline UniformReflection::ShaderKindSelector& UniformReflection::operator[] (const RG::RenderOperation& renderOp)
{
    return (*this)[renderOp.GetUUID ()];
}


inline UniformReflection::ShaderKindSelector& UniformReflection::operator[] (const Ptr<RG::RenderOperation>& renderOp)
{
    return (*this)[renderOp->GetUUID ()];
}


} // namespace RG

#endif