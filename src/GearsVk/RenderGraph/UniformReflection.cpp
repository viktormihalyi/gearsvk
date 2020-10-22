#include "UniformReflection.hpp"

#include "Event.hpp"


namespace RG {


UniformReflection::UniformReflection (RG::RenderGraph& graph, const Filter& filter, const ResourceCreator& resourceCreator)
    : graph (graph)
{
    // TODO properly handle swapchain recreate

    CreateGraphResources (filter, resourceCreator);
    CreateGraphConnections ();
}


void UniformReflection::Flush (uint32_t frameIndex)
{
    Utils::ForEachP<RG::CPUBufferResource> (uboResources, [&] (const RG::CPUBufferResourceP& uboRes) {
        const SR::IUDataP uboData = udatas.at (uboRes->GetUUID ());

        uboRes->GetMapping (frameIndex).Copy (uboData->GetData (), uboData->GetSize ());
    });
}


void UniformReflection::CreateGraphResources (const Filter& filter, const ResourceCreator& resourceCreator)
{
    GVK_ASSERT (!graph.operations.empty ());

    Utils::ForEachP<RG::RenderOperation> (graph.operations, [&] (const RG::RenderOperationP& renderOp) {
        ShaderKindSelector newsel;

        renderOp->compileSettings.pipeline->IterateShaders ([&] (const ShaderModule& shaderModule) {
            UboSelector ubosel;
            for (SR::UBOP ubo : shaderModule.GetReflection ().ubos) {
                if (filter (renderOp, shaderModule, ubo)) {
                    continue;
                }

                RG::InputBufferBindableResourceP uboRes = resourceCreator (renderOp, shaderModule, ubo);
                graph.AddResource (uboRes);
                GVK_ASSERT (uboRes != nullptr);

                SR::UDataInternalP uboData = SR::UDataInternal::Create (ubo);
                ubosel.Set (ubo->name, uboData);

                uboConnections.push_back (std::make_tuple (renderOp, ubo->binding, uboRes, shaderModule.GetShaderKind ()));
                uboResources.push_back (uboRes);
                udatas.insert ({ uboRes->GetUUID (), uboData });
            }
            newsel.Set (shaderModule.GetShaderKind (), std::move (ubosel));
        });
        selectors.emplace (renderOp->GetUUID (), std::move (newsel));
    });
}


void UniformReflection::CreateGraphConnections ()
{
    GVK_ASSERT (!uboConnections.empty ());

    const auto shaderKindToShaderStage = [] (ShaderKind shaderKind) -> VkShaderStageFlags {
        switch (shaderKind) {
            case ShaderKind::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderKind::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
            case ShaderKind::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
            case ShaderKind::TessellationControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            case ShaderKind::TessellationEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            case ShaderKind::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
            default: GVK_BREAK ("unexpected shaderkind type"); return VK_SHADER_STAGE_ALL;
        }
    };

    for (auto& [operation, binding, resource, shaderKind] : uboConnections) {
        graph.CreateInputConnection (*operation, *resource, RG::UniformInputBinding::Create (binding, *resource, shaderKindToShaderStage (shaderKind)));
    }

    uboConnections.clear ();
}


void UniformReflection::PrintDebugInfo ()
{
    for (auto& [uuid, selector] : selectors) {
        std::cout << uuid.GetValue () << std::endl;
        for (auto& [shaderKind, uboSelector] : selector.uboSelectors) {
            std::cout << "\t" << ShaderKindToString (shaderKind) << std::endl;
            for (auto& [name, ubo] : uboSelector.udatas) {
                std::cout << "\t\t" << name << " (" << static_cast<int32_t> (ubo->GetSize ()) << " bytes): ";

                for (int32_t i = ubo->GetSize () - 1; i >= 0; --i) {
                    printf ("%02x ", ubo->GetData ()[i]);
                }

                std::cout << std::endl;
            }
        }
    }
}


ImageMap::ImageMap () = default;


ReadOnlyImageResourceP ImageMap::FindByName (const std::string& name) const
{
    for (auto& im : images) {
        if (im.first.name == name) {
            return im.second;
        }
    }
    return nullptr;
}


void ImageMap::Put (const SR::Sampler& sampler, const ReadOnlyImageResourceP& res)
{
    images.emplace_back (sampler, res);
}


ImageMap CreateEmptyImageResources (RG::RenderGraph& graph)
{
    return CreateEmptyImageResources (graph, [&] (const SR::Sampler&) { return std::nullopt; });
}


ImageMap CreateEmptyImageResources (RG::RenderGraph& graph, const ExtentProviderForImageCreate& extentProvider)
{
    ImageMap result;

    Utils::ForEachP<RG::RenderOperation> (graph.operations, [&] (const RG::RenderOperationP& renderOp) {
        renderOp->compileSettings.pipeline->IterateShaders ([&] (const ShaderModule& shaderModule) {
            for (const SR::Sampler& sampler : shaderModule.GetReflection ().samplers) {
                ReadOnlyImageResourceP imgRes;

                const std::optional<CreateParams> providedExtent = extentProvider (sampler);

                const glm::uvec3 extent = providedExtent.has_value () ? std::get<0> (*providedExtent) : glm::uvec3 { 512, 512, 512 };
                const VkFormat   format = providedExtent.has_value () ? std::get<1> (*providedExtent) : VK_FORMAT_R8_SRGB;
                const VkFilter   filter = providedExtent.has_value () ? std::get<2> (*providedExtent) : VK_FILTER_LINEAR;

                switch (sampler.type) {
                    case SR::Sampler::Type::Sampler1D:
                        GVK_ASSERT (!providedExtent.has_value () || (extent.x != 0 && extent.y == 0 && extent.z == 0));
                        imgRes = graph.CreateResource<ReadOnlyImageResource> (format, filter, extent.x);
                        break;
                    case SR::Sampler::Type::Sampler2D:
                        GVK_ASSERT (!providedExtent.has_value () || (extent.x != 0 && extent.y != 0 && extent.z == 0));
                        imgRes = graph.CreateResource<ReadOnlyImageResource> (format, filter, extent.x, extent.y);
                        break;
                    case SR::Sampler::Type::Sampler3D:
                        GVK_ASSERT (!providedExtent.has_value () || (extent.x != 0 && extent.y != 0 && extent.z != 0));
                        imgRes = graph.CreateResource<ReadOnlyImageResource> (format, filter, extent.x, extent.y, extent.z);
                        break;
                    default:
                        GVK_BREAK ("unexpected sampler type");
                        break;
                }

                if (GVK_ERROR (imgRes == nullptr)) {
                    continue;
                }

                result.Put (sampler, imgRes);

                graph.CreateInputConnection (*renderOp, *imgRes, ImageInputBinding::Create (sampler.binding, *imgRes));
            }
        });
    });

    return result;
}

} // namespace RG
