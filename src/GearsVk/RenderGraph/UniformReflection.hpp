#ifndef RENDERGRAPH_UNIFORM_REFLECTION_HPP
#define RENDERGRAPH_UNIFORM_REFLECTION_HPP

#include "Operation.hpp"
#include "RenderGraph.hpp"
#include "Resource.hpp"
#include "ShaderReflection.hpp"

#include "Assert.hpp"
#include "GearsVkAPI.hpp"
#include "UUID.hpp"
#include "UniformView.hpp"
#include "glmlib.hpp"

#include <tuple>
#include <unordered_map>
#include <vector>

namespace GVK {

namespace RG {

// TODO add possibility to store uniforms in gpu memory

// adds resources to and existing rendergraph based on the renderoperations inside it
// modifying uniforms takes place in a staging cpu memory, calling Flush will copy these to the actual uniform memory
// accessing a uniforms is available with operator[] eg.: reflection[std::shared_ptr<RG::RenderOperation>][ShaderKind][std::string][std::string]...

class GVK_RENDERER_API ImageMap {
private:
    std::vector<std::pair<GVK::SR::Sampler, std::shared_ptr<ReadOnlyImageResource>>> images;

public:
    ImageMap ();

    std::shared_ptr<ReadOnlyImageResource> FindByName (const std::string& name) const;

    void Put (const GVK::SR::Sampler& sampler, const std::shared_ptr<ReadOnlyImageResource>& res);
};

using CreateParams                 = std::tuple<glm::uvec3, VkFormat, VkFilter>;
using ExtentProviderForImageCreate = std::function<std::optional<CreateParams> (const GVK::SR::Sampler& sampler)>;

GVK_RENDERER_API
ImageMap CreateEmptyImageResources (RG::ConnectionSet& connectionSet);

GVK_RENDERER_API
ImageMap CreateEmptyImageResources (RG::ConnectionSet& connectionSet, const ExtentProviderForImageCreate& extentProvider);


class GVK_RENDERER_API UniformReflection final : public EventObserver {
private:
    class GVK_RENDERER_API UboSelector {
    private:
        std::unordered_map<std::string, std::shared_ptr<GVK::SR::IUData>> udatas;

    public:
        GVK::SR::IUData& operator[] (const std::string& uboName)
        {
            auto it = udatas.find (uboName);
            if (it != udatas.end ()) {
                return *it->second;
            }

            GVK_ASSERT (false);
            return GVK::SR::dummyUData;
        }

        void Set (const std::string& uboName, const std::shared_ptr<GVK::SR::IUData>& uboData)
        {
            udatas[uboName] = uboData;
        }

        bool Contains (const std::string& uboName) const
        {
            return udatas.find (uboName) != udatas.end ();
        }

        friend class UniformReflection;
    };

    class GVK_RENDERER_API ShaderKindSelector {
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
    std::unordered_map<GVK::UUID, ShaderKindSelector> selectors;

    RG::ConnectionSet& connectionSet;

    //
    std::vector<std::shared_ptr<RG::InputBufferBindableResource>>                                                                              uboResources;
    std::vector<std::tuple<std::shared_ptr<RG::RenderOperation>, uint32_t, std::shared_ptr<RG::InputBufferBindableResource>, GVK::ShaderKind>> uboConnections;
    std::unordered_map<GVK::UUID, std::shared_ptr<GVK::SR::IUData>>                                                                            udatas;

public:
    using Filter          = std::function<bool (const std::shared_ptr<RG::RenderOperation>&, const ShaderModule&, const std::shared_ptr<GVK::SR::UBO>&)>;
    using ResourceCreator = std::function<std::shared_ptr<RG::InputBufferBindableResource> (const std::shared_ptr<RG::RenderOperation>&, const ShaderModule&, const std::shared_ptr<GVK::SR::UBO>&)>;

public:
    static bool DefaultFilter (const std::shared_ptr<RG::RenderOperation>&, const ShaderModule&, const std::shared_ptr<GVK::SR::UBO>&)
    {
        return false;
    }

    static std::shared_ptr<RG::InputBufferBindableResource> DefaultResourceCreator (const std::shared_ptr<RG::RenderOperation>&, const ShaderModule&, const std::shared_ptr<GVK::SR::UBO>& ubo)
    {
        return std::make_unique<RG::CPUBufferResource> (ubo->GetFullSize ());
    }

    static std::shared_ptr<RG::InputBufferBindableResource> GPUBufferResourceCreator (const std::shared_ptr<RG::RenderOperation>&, const ShaderModule&, const std::shared_ptr<GVK::SR::UBO>& ubo)
    {
        return std::make_unique<RG::GPUBufferResource> (ubo->GetFullSize ());
    }

public:
    UniformReflection (RG::ConnectionSet&     connectionSet,
                       const Filter&          filter          = &DefaultFilter,
                       const ResourceCreator& resourceCreator = &DefaultResourceCreator);

    void Flush (uint32_t frameIndex);

    ShaderKindSelector& operator[] (const RG::RenderOperation& renderOp);
    ShaderKindSelector& operator[] (const std::shared_ptr<RG::RenderOperation>& renderOp);
    ShaderKindSelector& operator[] (const GVK::UUID& renderOpId);

private:
    void CreateGraphResources (const Filter& filter, const ResourceCreator& resourceCreator);

    void CreateGraphConnections ();

public:
    void PrintDebugInfo ();
};


inline UniformReflection::ShaderKindSelector& UniformReflection::operator[] (const GVK::UUID& renderOpId)
{
    GVK_ASSERT (selectors.find (renderOpId) != selectors.end ());
    return selectors.at (renderOpId);
}


inline UniformReflection::ShaderKindSelector& UniformReflection::operator[] (const RG::RenderOperation& renderOp)
{
    return (*this)[renderOp.GetUUID ()];
}


inline UniformReflection::ShaderKindSelector& UniformReflection::operator[] (const std::shared_ptr<RG::RenderOperation>& renderOp)
{
    return (*this)[renderOp->GetUUID ()];
}


} // namespace RG

} // namespace GVK

#endif