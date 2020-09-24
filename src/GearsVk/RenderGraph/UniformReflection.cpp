#include "UniformReflection.hpp"

#include "Event.hpp"

namespace RG {


UniformReflection::UniformReflection (RG::RenderGraph& graph, const RG::GraphSettings& settings, const Filter& filter, const ResourceCreator& resourceCreator)
    : graph (graph)
    , settings (settings)
{
    CreateGraphResources (filter, resourceCreator);
    CreateGraphConnections ();

    // memory allocations and mappings are only created when the graph is compiled
    Observe<> (graph.compileEvent, [&] (void) {
        UniformReflection::RecordCopyOperations ();
    });
}


void UniformReflection::RecordCopyOperations ()
{
    GVK_ASSERT (!uboResources.empty ());
    GVK_ASSERT (!udatas.empty ());

    for (auto& op : copyOperations) {
        op.clear ();
    }

    for (const RG::ResourceP& res : uboResources) {
        if (RG::UniformBlockResourceP uboRes = std::dynamic_pointer_cast<RG::UniformBlockResource> (res)) {
            for (uint32_t frameIndex = 0; frameIndex < settings.framesInFlight; ++frameIndex) {
                const SR::IUDataP uboData = udatas.at (uboRes->GetUUID ());

                GVK_ASSERT (uboRes->mappings[frameIndex]->GetSize () == uboData->GetSize ());

                copyOperations[frameIndex].push_back (CopyOperation {
                    uboRes->mappings[frameIndex]->Get (),
                    uboData->GetData (),
                    uboData->GetSize () });
            }
        }
    }

    uboResources.clear ();
    udatas.clear ();
}

void UniformReflection::Flush (uint32_t frameIndex)
{
    GVK_ASSERT (!copyOperations.empty ());

    for (auto& copy : copyOperations[frameIndex]) {
        copy.Do ();
    }
}


static void ForEachRenderOperation (RG::RenderGraph& renderGraph, const std::function<void (const RG::RenderOperationP&)>& processor)
{
    for (const RG::OperationP& graphOperation : renderGraph.operations) {
        if (RG::RenderOperationP renderOp = std::dynamic_pointer_cast<RG::RenderOperation> (graphOperation)) {
            processor (renderOp);
        }
    }
}


void UniformReflection::CreateGraphResources (const Filter& filter, const ResourceCreator& resourceCreator)
{
    copyOperations.resize (settings.framesInFlight);

    GVK_ASSERT (!graph.operations.empty ());

    ForEachRenderOperation (graph, [&] (const RG::RenderOperationP& renderOp) {
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

                uboConnections.push_back (std::make_tuple (renderOp, ubo->binding, uboRes));
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

    for (auto& [operation, binding, resource] : uboConnections) {
        graph.CreateInputConnection (*operation, *resource, RG::UniformInputBinding::Create (binding, *resource));
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

                for (size_t i = 0; i < ubo->GetSize (); ++i) {
                    printf ("%02x ", ubo->GetData ()[i]);
                }

                std::cout << std::endl;
            }
        }
    }
}


ImageAdder::ImageAdder (RG::RenderGraph& graph, const RG::GraphSettings& settings)
{
    ForEachRenderOperation (graph, [&] (const RG::RenderOperationP& renderOp) {
        renderOp->compileSettings.pipeline->IterateShaders ([&] (const ShaderModule& shaderModule) {
            for (const SR::Sampler& sampler : shaderModule.GetReflection ().samplers) {
                ReadOnlyImageResourceP imgRes;

                switch (sampler.type) {
                    case SR::Sampler::Type::Sampler1D:
                        imgRes = graph.CreateResource<ReadOnlyImageResource> (VK_FORMAT_R8G8B8A8_SRGB, 512);
                        break;
                    case SR::Sampler::Type::Sampler2D:
                        imgRes = graph.CreateResource<ReadOnlyImageResource> (VK_FORMAT_R8G8B8A8_SRGB, 512, 512);
                        break;
                    case SR::Sampler::Type::Sampler3D:
                        imgRes = graph.CreateResource<ReadOnlyImageResource> (VK_FORMAT_R8_SRGB, 512, 512, 512);
                        break;
                    default:
                        GVK_BREAK ("unexpected sampler type");
                        break;
                }

                if (GVK_ERROR (imgRes == nullptr)) {
                    continue;
                }

                graph.CreateInputConnection (*renderOp, *imgRes, ImageInputBinding::Create (sampler.binding, *imgRes));
            }
        });
    });
}

} // namespace RG
