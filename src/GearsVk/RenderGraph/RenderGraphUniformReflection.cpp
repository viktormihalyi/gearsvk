#include "RenderGraphUniformReflection.hpp"


namespace RG {

RenderGraphUniformReflection::RenderGraphUniformReflection (RG::RenderGraph& graph, const RG::GraphSettings& settings, const Filter& filter, const ResourceCreator& resourceCreator)
    : graph (graph)
    , settings (settings)
{
    CreateGraphResources (filter, resourceCreator);
    CreateGraphConnections ();
}


void RenderGraphUniformReflection::RecordCopyOperations ()
{
    GVK_ASSERT (!uboResources.empty ());
    GVK_ASSERT (!udatas.empty ());

    for (const RG::ResourceP& res : uboResources) {
        if (RG::UniformBlockResourceP uboRes = std::dynamic_pointer_cast<RG::UniformBlockResource> (res)) {
            for (uint32_t frameIndex = 0; frameIndex < settings.framesInFlight; ++frameIndex) {
                // TODO mappings are only available after compile
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

void RenderGraphUniformReflection::Flush (uint32_t frameIndex)
{
    GVK_ASSERT (!copyOperations.empty ());

    for (auto& copy : copyOperations[frameIndex]) {
        copy.Do ();
    }
}


void RenderGraphUniformReflection::CreateGraphResources (const Filter& filter, const ResourceCreator& resourceCreator)
{
    copyOperations.resize (settings.framesInFlight);

    GVK_ASSERT (!graph.operations.empty ());

    for (const RG::OperationP& graphOperation : graph.operations) {
        if (RG::RenderOperationP renderOp = std::dynamic_pointer_cast<RG::RenderOperation> (graphOperation)) {
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

                    // TODO connection
                    SR::UDataInternalP uboData = SR::UDataInternal::Create (ubo);
                    ubosel.Set (ubo->name, uboData);

                    uboConnections.push_back (std::make_tuple (renderOp, ubo->binding, uboRes));
                    uboResources.push_back (uboRes);
                    udatas.insert ({ uboRes->GetUUID (), uboData });

                }
                newsel.Set (shaderModule.GetShaderKind (), std::move (ubosel));
            });
            selectors.emplace (renderOp->GetUUID (), std::move (newsel));
        }
    }
}


void RenderGraphUniformReflection::CreateGraphConnections ()
{
    GVK_ASSERT (!uboConnections.empty ());

    for (auto& [operation, binding, resource] : uboConnections) {
        graph.CreateInputConnection (*operation, *resource, RG::UniformInputBinding::Create (binding, *resource));
    }

    uboConnections.clear ();
}

} // namespace RG
