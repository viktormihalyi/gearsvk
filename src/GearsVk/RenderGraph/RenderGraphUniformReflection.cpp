#include "RenderGraphUniformReflection.hpp"


namespace RG {

RenderGraphUniformReflection::RenderGraphUniformReflection (RG::RenderGraph& graph, const RG::GraphSettings& settings)
    : graph (graph)
    , settings (settings)
{
    CreateGraphResources ();
    CreateGraphConnections ();
}


void RenderGraphUniformReflection::RecordCopyOperations ()
{
    GVK_ASSERT (!uboResources.empty ());
    GVK_ASSERT (!udatas.empty ());

    for (const RG::UniformBlockResourceP& uboRes : uboResources) {
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

    uboResources.clear ();
    udatas.clear ();
}

void RenderGraphUniformReflection::Flush (uint32_t frameIndex)
{
    for (auto& copy : copyOperations[frameIndex]) {
        copy.Do ();
    }
}


void RenderGraphUniformReflection::CreateGraphResources ()
{
    copyOperations.resize (settings.framesInFlight);

    GVK_ASSERT (!graph.operations.empty ());

    for (const RG::OperationP& graphOperation : graph.operations) {
        if (RG::RenderOperationP renderOp = std::dynamic_pointer_cast<RG::RenderOperation> (graphOperation)) {
            ShaderKindSelector newsel;

            renderOp->compileSettings.pipeline->IterateShaders ([&] (const ShaderModule& shaderModule) {
                UboSelector ubosel;
                for (SR::UBOP ubo : shaderModule.GetReflection ().ubos) {
                    RG::UniformBlockResourceP uboRes = graph.CreateResource<RG::UniformBlockResource> (*ubo);
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
