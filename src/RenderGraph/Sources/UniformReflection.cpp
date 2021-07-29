#include "UniformReflection.hpp"
#include "DrawRecordable.hpp"

#include "Utils/Event.hpp"

namespace GVK {

namespace RG {


UniformReflection::UniformReflection (RG::ConnectionSet& connectionSet, const Filter& filter, const ResourceCreator& resourceCreator)
    : connectionSet (connectionSet)
{
    // TODO properly handle swapchain recreate

    CreateGraphResources (filter, resourceCreator);
    CreateGraphConnections ();
}


void UniformReflection::Flush (uint32_t frameIndex)
{
    Utils::ForEach<RG::CPUBufferResource> (uboResources, [&] (const std::shared_ptr<RG::CPUBufferResource>& uboRes) {
        const std::shared_ptr<SR::IUData> uboData = udatas.at (uboRes->GetUUID ());

        uboRes->GetMapping (frameIndex).Copy (uboData->GetData (), uboData->GetSize ());
    });
}


void UniformReflection::CreateGraphResources (const Filter& filter, const ResourceCreator& resourceCreator)
{
    // GVK_ASSERT (!graph.operations.empty ());

    Utils::ForEach<RG::RenderOperation> (connectionSet.insertionOrder, [&] (const std::shared_ptr<RG::RenderOperation>& renderOp) {
        ShaderKindSelector newsel;

        renderOp->GetShaderPipeline ()->IterateShaders ([&] (const ShaderModule& shaderModule) {
            UboSelector ubosel;
            for (std::shared_ptr<SR::UBO> ubo : shaderModule.GetReflection ().ubos) {
                if (filter (renderOp, shaderModule, ubo)) {
                    continue;
                }

                std::shared_ptr<RG::InputBufferBindableResource> uboRes = resourceCreator (renderOp, shaderModule, ubo);

                // graph.AddResource (uboRes);
                GVK_ASSERT (uboRes != nullptr);

                uboRes->SetName (ubo->name);
                uboRes->SetDebugInfo ("Made by UniformReflection.");

                std::shared_ptr<SR::UDataInternal> uboData = std::make_unique<SR::UDataInternal> (ubo);
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
        connectionSet.Add (resource, operation, std::make_unique<RG::UniformInputBinding> (binding, *resource, shaderKindToShaderStage (shaderKind)));
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


std::shared_ptr<ReadOnlyImageResource> ImageMap::FindByName (const std::string& name) const
{
    for (auto& im : images) {
        if (im.first.name == name) {
            return im.second;
        }
    }
    return nullptr;
}


void ImageMap::Put (const SR::Sampler& sampler, const std::shared_ptr<ReadOnlyImageResource>& res)
{
    images.emplace_back (sampler, res);
}


ImageMap CreateEmptyImageResources (RG::ConnectionSet& connectionSet)
{
    return CreateEmptyImageResources (connectionSet, [&] (const SR::Sampler&) { return std::nullopt; });
}


ImageMap CreateEmptyImageResources (RG::ConnectionSet& connectionSet, const ExtentProviderForImageCreate& extentProvider)
{
    ImageMap result;

    const auto nodes = connectionSet.insertionOrder;
    Utils::ForEach<RG::RenderOperation> (nodes, [&] (const std::shared_ptr<RG::RenderOperation>& renderOp) {
        renderOp->GetShaderPipeline ()->IterateShaders ([&] (const ShaderModule& shaderModule) {
            for (const SR::Sampler& sampler : shaderModule.GetReflection ().samplers) {
                std::shared_ptr<ReadOnlyImageResource> imgRes;

                const std::optional<CreateParams> providedExtent = extentProvider (sampler);
                if (!providedExtent.has_value ()) {
                    continue;
                }

                const glm::uvec3 extent = providedExtent.has_value () ? std::get<0> (*providedExtent) : glm::uvec3 { 512, 512, 512 };
                const VkFormat   format = providedExtent.has_value () ? std::get<1> (*providedExtent) : VK_FORMAT_R8_SRGB;
                const VkFilter   filter = providedExtent.has_value () ? std::get<2> (*providedExtent) : VK_FILTER_LINEAR;

                const uint32_t layerCount = sampler.arraySize == 0 ? 1 : sampler.arraySize;

                switch (sampler.type) {
                    case SR::Sampler::Type::Sampler1D:
                        GVK_ASSERT (!providedExtent.has_value () || (extent.x != 0 && extent.y == 0 && extent.z == 0));
                        imgRes = std::make_unique<ReadOnlyImageResource> (format, filter, extent.x, 1, 1, layerCount);
                        break;
                    case SR::Sampler::Type::Sampler2D:
                        GVK_ASSERT (!providedExtent.has_value () || (extent.x != 0 && extent.y != 0 && extent.z == 0));
                        imgRes = std::make_unique<ReadOnlyImageResource> (format, filter, extent.x, extent.y, 1, layerCount);
                        break;
                    case SR::Sampler::Type::Sampler3D:
                        GVK_ASSERT (!providedExtent.has_value () || (extent.x != 0 && extent.y != 0 && extent.z != 0));
                        imgRes = std::make_unique<ReadOnlyImageResource> (format, filter, extent.x, extent.y, extent.z, layerCount);
                        break;
                    default:
                        GVK_BREAK ("unexpected sampler type");
                        break;
                }

                if (GVK_ERROR (imgRes == nullptr)) {
                    continue;
                }

                imgRes->SetName (sampler.name);
                imgRes->SetDebugInfo ("Made by ImageMap.");

                result.Put (sampler, imgRes);

                connectionSet.Add (imgRes, renderOp, std::make_unique<ImageInputBinding> (sampler.binding, *imgRes, (sampler.arraySize > 0) ? sampler.arraySize : 1));
            }
        });
    });

    return result;
}

} // namespace RG

} // namespace GVK
