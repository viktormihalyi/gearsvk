#include "UniformReflection.hpp"
#include "DrawRecordable.hpp"
#include "ShaderPipeline.hpp"
#include "ComputeShaderPipeline.hpp"
#include "Operation.hpp"
#include "ShaderReflectionToDescriptor.hpp"

#include "VulkanWrapper/Framebuffer.hpp"
#include "VulkanWrapper/GraphicsPipeline.hpp"
#include "VulkanWrapper/ComputePipeline.hpp"
#include "VulkanWrapper/PipelineLayout.hpp"
#include "VulkanWrapper/RenderPass.hpp"
#include "VulkanWrapper/PipelineLayout.hpp"
#include "VulkanWrapper/DescriptorSet.hpp"
#include "VulkanWrapper/DescriptorSetLayout.hpp"

#include "Utils/Event.hpp"
#include "Utils/Utils.hpp"

#include "spdlog/spdlog.h"

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

        renderOp->GetShaderPipeline ()->IterateShaders ([&] (const GVK::ShaderModule& shaderModule) {
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

                uboConnections.push_back (std::make_tuple (renderOp, ubo, uboRes, shaderModule.GetShaderKind ()));
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

    const auto shaderKindToShaderStage = [] (GVK::ShaderKind shaderKind) -> VkShaderStageFlags {
        switch (shaderKind) {
            case GVK::ShaderKind::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
            case GVK::ShaderKind::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
            case GVK::ShaderKind::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
            case GVK::ShaderKind::TessellationControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            case GVK::ShaderKind::TessellationEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            case GVK::ShaderKind::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
            default: GVK_BREAK_STR ("unexpected shaderkind type"); return VK_SHADER_STAGE_ALL;
        }
    };

    for (auto& [operation, ubo, resource, shaderKind] : uboConnections) {
        connectionSet.Add (operation);
        connectionSet.Add (resource);
        
        auto& table = operation->compileSettings.descriptorWriteProvider;
        table->bufferInfos.push_back ({ ubo->name, shaderKind, resource->GetBufferForFrameProvider (), 0, resource->GetBufferSize () });
    }

    uboConnections.clear ();
}


void UniformReflection::PrintDebugInfo ()
{
    for (auto& [uuid, selector] : selectors) {
        spdlog::info ("{}", uuid.GetValue ());
        for (auto& [shaderKind, uboSelector] : selector.uboSelectors) {
            spdlog::info ("\t{}", ShaderKindToString (shaderKind));
            for (auto& [name, ubo] : uboSelector.udatas) {
                spdlog::info ("\t\t{} ({})", name, ubo->GetSize ());

                for (int32_t i = ubo->GetSize () - 1; i >= 0; --i) {
                    spdlog::info ("%02x ", ubo->GetData ()[i]); // TODO
                }
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
        renderOp->GetShaderPipeline ()->IterateShaders ([&] (const GVK::ShaderModule& shaderModule) {
            for (const SR::Sampler& sampler : shaderModule.GetReflection ().samplers) {
                std::shared_ptr<ReadOnlyImageResource> imgRes;

                const std::optional<CreateParams> providedExtent = extentProvider (sampler);
                if (!providedExtent.has_value ()) {
                    continue;
                }

                const glm::uvec3 extent = providedExtent.has_value () ? std::get<0> (*providedExtent) : glm::uvec3 { 512, 512, 512 };
                const VkFormat   format = providedExtent.has_value () ? std::get<1> (*providedExtent) : VK_FORMAT_R8_SRGB;
                const VkFilter   filter = providedExtent.has_value () ? std::get<2> (*providedExtent) : VK_FILTER_LINEAR;

                const uint32_t layerCount = sampler.arraySize;

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
                        GVK_BREAK_STR ("unexpected sampler type");
                        break;
                }

                if (GVK_ERROR (imgRes == nullptr)) {
                    continue;
                }

                imgRes->SetName (sampler.name);
                imgRes->SetDebugInfo ("Made by ImageMap.");

                result.Put (sampler, imgRes);

                connectionSet.Add (imgRes);
                connectionSet.Add (renderOp);
                auto& table = renderOp->compileSettings.descriptorWriteProvider;
                table->imageInfos.push_back ({ sampler.name, shaderModule.GetShaderKind (), imgRes->GetSamplerProvider (), imgRes->GetImageViewForFrameProvider (), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
            }
        });
    });

    return result;
}

} // namespace RG
