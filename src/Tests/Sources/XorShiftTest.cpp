#include "TestEnvironment.hpp"

#include "RenderGraph/DrawRecordable/DrawRecordableInfo.hpp"
#include "RenderGraph/GraphSettings.hpp"
#include "RenderGraph/Operation.hpp"
#include "RenderGraph/RenderGraph.hpp"
#include "RenderGraph/Resource.hpp"
#include "RenderGraph/ShaderPipeline.hpp"
#include "RenderGraph/VulkanEnvironment.hpp"
#include "RenderGraph/Window/Window.hpp"
#include "RenderGraph/ConnectionBuilder.hpp"

#include "VulkanWrapper/RenderPass.hpp"
#include "VulkanWrapper/Pipeline.hpp"
#include "VulkanWrapper/PipelineLayout.hpp"
#include "VulkanWrapper/ShaderModule.hpp"
#include "VulkanWrapper/DeviceExtra.hpp"
#include "VulkanWrapper/Utils/ImageData.hpp"

#include <memory>
#include <string>

TEST_F (HeadlessTestEnvironment, DISABLED_XorShiftRNG)
{
    const std::string fragSrc = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding = 2) uniform CommonUniforms {
    uint randomTextureIndex;
};

layout (location = 0) in vec2 textureCoords;

layout (binding = 8) uniform sampler2D randomTextureIn[5];

void main () {
    // outColor = texture (randomTextureIn[randomTextureIndex], textureCoords);
}
    )";

    const std::string fragSrc2 = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding = 2) uniform CommonUniforms {
    uint randomTextureIndex;
};

layout (binding = 8) uniform sampler2D randomTextureIn[5];

layout (location = 0) out vec4 randomTextureOut[5];

void main () {
    // outColor = vec4 (1, 0, 0, 0.5);
}
    )";

    std::shared_ptr<RG::SingleWritableImageResource> renderTarget = std::make_shared<RG::SingleWritableImageResource> (VK_FILTER_LINEAR, 512, 512, 1, VK_FORMAT_R8G8B8A8_SRGB);


    std::shared_ptr<RG::RenderOperation> renderOp = RG::RenderOperation::Builder (GetDevice ())
                                                        .SetVertices (std::make_unique<RG::DrawRecordableInfo> (1, 6))
                                                        .SetPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                                                        .SetVertexShader (passThroughVertexShader)
                                                        .SetFragmentShader (fragSrc)
                                                        .Build ();

    std::shared_ptr<RG::RenderOperation> renderOp2 = RG::RenderOperation::Builder (GetDevice ())
                                                        .SetVertices (std::make_unique<RG::DrawRecordableInfo> (1, 6))
                                                        .SetPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                                                        .SetVertexShader (passThroughVertexShader)
                                                        .SetFragmentShader (fragSrc2)
                                                        .Build ();

    RG::ConnectionSet connectionSet;

    connectionSet.Add (RG::OutputBuilder ()
            .SetOperation (renderOp)
            .SetTarget (renderTarget)
            .SetBinding (0)
            .SetClear ()
            .Build ());
    
    connectionSet.Add (RG::OutputBuilder ()
            .SetOperation (renderOp2)
            .SetTarget (renderTarget)
            .SetBinding (1)
            .SetClear ()
            .Build ());

    RG::GraphSettings s;
    s.connectionSet = std::move (connectionSet);
    s.framesInFlight = 3;
    s.device         = env->deviceExtra.get ();
    
    RG::RenderGraph rg;
    rg.Compile (std::move (s));

    VkClearValue clearValue     = {};
    clearValue.color.float32[0] = 0.0f;
    clearValue.color.float32[1] = 0.0f;
    clearValue.color.float32[2] = 0.0f;
    clearValue.color.float32[3] = 1.0f;

    VkMemoryBarrier flushAllMemory = {};
    flushAllMemory.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    flushAllMemory.srcAccessMask   = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
                                   VK_ACCESS_INDEX_READ_BIT |
                                   VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                                   VK_ACCESS_UNIFORM_READ_BIT |
                                   VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_SHADER_READ_BIT |
                                   VK_ACCESS_SHADER_WRITE_BIT |
                                   VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                   VK_ACCESS_TRANSFER_READ_BIT |
                                   VK_ACCESS_TRANSFER_WRITE_BIT;
    flushAllMemory.dstAccessMask = flushAllMemory.srcAccessMask;


    std::vector<GVK::CommandBuffer> commandBuffers;

    const VkImageMemoryBarrier transition = renderTarget->images[0]->image->GetBarrier (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                                        flushAllMemory.srcAccessMask,
                                                                                        flushAllMemory.srcAccessMask);

    {
        GVK::CommandBuffer commandBuffer (*env->device, *env->commandPool);

        commandBuffer.Begin ();
        commandBuffer.Record<GVK::CommandPipelineBarrier> (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, std::vector<VkMemoryBarrier> { flushAllMemory }, std::vector<VkBufferMemoryBarrier> {}, std::vector<VkImageMemoryBarrier> { transition });
        commandBuffer.Record<GVK::CommandBeginRenderPass> (*renderOp->compileSettings.pipeline->compileResult.renderPass, *renderOp->compileResult.framebuffers[0], VkRect2D { { 0, 0 }, { 512, 512 } }, std::vector<VkClearValue> { clearValue }, VK_SUBPASS_CONTENTS_INLINE);
        commandBuffer.Record<GVK::CommandBindPipeline> (VK_PIPELINE_BIND_POINT_GRAPHICS, *renderOp->compileSettings.pipeline->compileResult.pipeline);
        commandBuffer.Record<GVK::CommandDraw> (6, 1, 0, 0);
        commandBuffer.Record<GVK::CommandEndRenderPass> ();
        commandBuffer.Record<GVK::CommandPipelineBarrier> (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, std::vector<VkMemoryBarrier> { flushAllMemory }, std::vector<VkBufferMemoryBarrier> {}, std::vector<VkImageMemoryBarrier> { transition });
        commandBuffer.Record<GVK::CommandBeginRenderPass> (*renderOp2->compileSettings.pipeline->compileResult.renderPass, *renderOp2->compileResult.framebuffers[0], VkRect2D { { 0, 0 }, { 512, 512 } }, std::vector<VkClearValue> { clearValue }, VK_SUBPASS_CONTENTS_INLINE);
        commandBuffer.Record<GVK::CommandBindPipeline> (VK_PIPELINE_BIND_POINT_GRAPHICS, *renderOp2->compileSettings.pipeline->compileResult.pipeline);
        commandBuffer.Record<GVK::CommandDraw> (6, 1, 0, 0);
        commandBuffer.Record<GVK::CommandEndRenderPass> ();
        commandBuffer.Record<GVK::CommandPipelineBarrier> (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, std::vector<VkMemoryBarrier> { flushAllMemory }, std::vector<VkBufferMemoryBarrier> {}, std::vector<VkImageMemoryBarrier> {});
        commandBuffer.End ();

        commandBuffers.push_back (std::move (commandBuffer));
    }

    ASSERT_TRUE (commandBuffers[0].recordedAbstractCommands.size () == rg.commandBuffers[0].recordedAbstractCommands.size ());
    for (size_t i = 0; i < rg.commandBuffers[0].recordedAbstractCommands.size (); ++i) {
        EXPECT_TRUE (rg.commandBuffers[0].recordedAbstractCommands[i]->IsEquivalent (*commandBuffers[0].recordedAbstractCommands[i])) << i;
    }

    //env->graphicsQueue->Submit ({}, {}, commandBuffers, {}, VK_NULL_HANDLE);
    env->graphicsQueue->Submit ({}, {}, rg.commandBuffers, {}, VK_NULL_HANDLE);

    env->graphicsQueue->Wait ();

    GVK::ImageData img (GetDeviceExtra (), *renderTarget->images[0]->image, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    GVK::ImageData refimg (ReferenceImagesFolder / "pink.png");

    EXPECT_TRUE (refimg == img);
}