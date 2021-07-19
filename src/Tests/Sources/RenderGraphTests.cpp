
// from RenderGraph
#include "RenderGraph/DrawRecordable/FullscreenQuad.hpp"
#include "RenderGraph/GraphRenderer.hpp"
#include "RenderGraph/GraphSettings.hpp"
#include "VulkanWrapper/Utils/ImageData.hpp"
#include "RenderGraph/Operation.hpp"
#include "RenderGraph/RenderGraph.hpp"
#include "RenderGraph/Resource.hpp"
#include "RenderGraph/ShaderPipeline.hpp"
#include "RenderGraph/UniformReflection.hpp"
#include "RenderGraph/UniformView.hpp"
#include "RenderGraph/VulkanEnvironment.hpp"
#include "VulkanWrapper/Utils/VulkanUtils.hpp"
#include "VulkanWrapper/VulkanWrapper.hpp"
#include "VulkanWrapper/Allocator.hpp"
#include "VulkanWrapper/DeviceExtra.hpp"
#include <glm/glm.hpp>

// from std
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>

// from Testing
#include "GoogleTestEnvironment.hpp"

// from Utils
#include "Utils/SourceLocation.hpp"
#include "Utils/Timer.hpp"
#include "Utils/Utils.hpp"

// from vulkan
#include <vulkan/vulkan.h>


const std::filesystem::path ShadersFolder = std::filesystem::current_path () / "TestData" / "shaders";


using Empty = ::testing::Test;

TEST_F (Empty, RenderGraphPassTest_SingleOutput)
{
    using namespace GVK::RG;

    Operation* op1  = reinterpret_cast<Operation*> (1);
    Resource*  res1 = reinterpret_cast<Resource*> (2);

    GVK::RG::Pass p;
    p.AddOutput (op1, res1);

    EXPECT_EQ (0, p.GetAllInputs ().size ());
    EXPECT_EQ (1, p.GetAllOutputs ().size ());
    EXPECT_EQ (1, p.GetAllOperations ().size ());

    EXPECT_EQ (res1, p.GetResourceIO (res1)->res);
    EXPECT_EQ (op1, p.GetOperationIO (op1)->op);
    EXPECT_EQ (1, p.GetOperationIO (op1)->outputs.size ());
    EXPECT_EQ (0, p.GetOperationIO (op1)->inputs.size ());
    
}


TEST_F (Empty, RenderGraphPassTest_SingleInput)
{
    using namespace GVK::RG;

    Operation* op1  = reinterpret_cast<Operation*> (1);
    Resource*  res1 = reinterpret_cast<Resource*> (2);

    GVK::RG::Pass p;
    p.AddInput (op1, res1);

    EXPECT_EQ (1, p.GetAllInputs ().size ());
    EXPECT_EQ (0, p.GetAllOutputs ().size ());
    EXPECT_EQ (1, p.GetAllOperations ().size ());

    EXPECT_EQ (res1, p.GetResourceIO (res1)->res);
    EXPECT_EQ (op1, p.GetOperationIO (op1)->op);
    EXPECT_EQ (0, p.GetOperationIO (op1)->outputs.size ());
    EXPECT_EQ (1, p.GetOperationIO (op1)->inputs.size ());
}


TEST_F (Empty, RenderGraphPassTest_SingleOutput_Remove)
{
    using namespace GVK::RG;

    Operation* op1  = reinterpret_cast<Operation*> (1);
    Resource*  res1 = reinterpret_cast<Resource*> (2);

    GVK::RG::Pass p;
    p.AddOutput (op1, res1);
    p.RemoveOutput (op1, res1);

    EXPECT_EQ (0, p.GetAllInputs ().size ());
    EXPECT_EQ (0, p.GetAllOutputs ().size ());
    EXPECT_EQ (0, p.GetAllOperations ().size ());
}


TEST_F (Empty, RenderGraphPassTest_SingleInput_Remove)
{
    using namespace GVK::RG;

    Operation* op1  = reinterpret_cast<Operation*> (1);
    Resource*  res1 = reinterpret_cast<Resource*> (2);

    GVK::RG::Pass p;
    p.AddInput (op1, res1);
    p.RemoveInput (op1, res1);

    EXPECT_EQ (0, p.GetAllInputs ().size ());
    EXPECT_EQ (0, p.GetAllOutputs ().size ());
    EXPECT_EQ (0, p.GetAllOperations ().size ());
}


TEST_F (Empty, RenderGraphPassTest_MultipleInput)
{
    using namespace GVK::RG;

    Operation* op1  = reinterpret_cast<Operation*> (1);
    Resource*  res1 = reinterpret_cast<Resource*> (2);
    Resource*  res2 = reinterpret_cast<Resource*> (3);

    GVK::RG::Pass p;
    p.AddInput (op1, res1);
    p.AddInput (op1, res2);

    EXPECT_EQ (2, p.GetAllInputs ().size ());
    EXPECT_EQ (0, p.GetAllOutputs ().size ());
    EXPECT_EQ (1, p.GetAllOperations ().size ());
}


TEST_F (Empty, RenderGraphPassTest_MultipleInput_RemoveOne)
{
    using namespace GVK::RG;

    Operation* op1  = reinterpret_cast<Operation*> (1);
    Resource*  res1 = reinterpret_cast<Resource*> (2);
    Resource*  res2 = reinterpret_cast<Resource*> (3);

    GVK::RG::Pass p;
    p.AddInput (op1, res1);
    p.AddInput (op1, res2);
    p.RemoveInput (op1, res2);

    EXPECT_EQ (1, p.GetAllInputs ().size ());
    EXPECT_EQ (0, p.GetAllOutputs ().size ());
    EXPECT_EQ (1, p.GetAllOperations ().size ());

    EXPECT_EQ (res1, p.GetResourceIO (res1)->res);
    EXPECT_EQ (op1, p.GetOperationIO (op1)->op);
    EXPECT_EQ (0, p.GetOperationIO (op1)->outputs.size ());
    EXPECT_EQ (1, p.GetOperationIO (op1)->inputs.size ());
}


TEST_F (Empty, RenderGraphPassTest_MultipleInput_RemoveAll)
{
    using namespace GVK::RG;

    Operation* op1  = reinterpret_cast<Operation*> (1);
    Resource*  res1 = reinterpret_cast<Resource*> (2);
    Resource*  res2 = reinterpret_cast<Resource*> (3);
    Resource*  res3 = reinterpret_cast<Resource*> (4);

    GVK::RG::Pass p;
    p.AddInput (op1, res1);
    p.AddInput (op1, res2);
    p.AddInput (op1, res3);
    p.RemoveInput (op1, res2);
    p.RemoveInput (op1, res1);
    p.RemoveInput (op1, res3);

    EXPECT_EQ (0, p.GetAllInputs ().size ());
    EXPECT_EQ (0, p.GetAllOutputs ().size ());
    EXPECT_EQ (0, p.GetAllOperations ().size ());
}


TEST_F (Empty, RenderGraphPassTest_MultipleOutput)
{
    using namespace GVK::RG;

    Operation* op1  = reinterpret_cast<Operation*> (1);
    Resource*  res1 = reinterpret_cast<Resource*> (2);
    Resource*  res2 = reinterpret_cast<Resource*> (3);

    GVK::RG::Pass p;
    p.AddOutput (op1, res1);
    p.AddOutput (op1, res2);

    EXPECT_EQ (2, p.GetAllOutputs ().size ());
    EXPECT_EQ (0, p.GetAllInputs ().size ());
    EXPECT_EQ (1, p.GetAllOperations ().size ());
}


TEST_F (Empty, RenderGraphPassTest_MultipleOutput_RemoveOne)
{
    using namespace GVK::RG;

    Operation* op1  = reinterpret_cast<Operation*> (1);
    Resource*  res1 = reinterpret_cast<Resource*> (2);
    Resource*  res2 = reinterpret_cast<Resource*> (3);

    GVK::RG::Pass p;
    p.AddOutput (op1, res1);
    p.AddOutput (op1, res2);
    p.RemoveOutput (op1, res2);

    EXPECT_EQ (1, p.GetAllOutputs ().size ());
    EXPECT_EQ (0, p.GetAllInputs ().size ());
    EXPECT_EQ (1, p.GetAllOperations ().size ());

    EXPECT_EQ (res1, p.GetResourceIO (res1)->res);
    EXPECT_EQ (op1, p.GetOperationIO (op1)->op);
    EXPECT_EQ (1, p.GetOperationIO (op1)->outputs.size ());
    EXPECT_EQ (0, p.GetOperationIO (op1)->inputs.size ());
}


TEST_F (Empty, RenderGraphPassTest_MultipleOutput_RemoveAll)
{
    using namespace GVK::RG;

    Operation* op1  = reinterpret_cast<Operation*> (1);
    Resource*  res1 = reinterpret_cast<Resource*> (2);
    Resource*  res2 = reinterpret_cast<Resource*> (3);
    Resource*  res3 = reinterpret_cast<Resource*> (4);

    GVK::RG::Pass p;
    p.AddOutput (op1, res1);
    p.AddOutput (op1, res2);
    p.AddOutput (op1, res3);
    p.RemoveOutput (op1, res2);
    p.RemoveOutput (op1, res1);
    p.RemoveOutput (op1, res3);

    EXPECT_EQ (0, p.GetAllOutputs ().size ());
    EXPECT_EQ (0, p.GetAllInputs ().size ());
    EXPECT_EQ (0, p.GetAllOperations ().size ());
}


TEST_F (Empty, RenderGraphPassTest_MultipleIO)
{
    using namespace GVK::RG;

    Operation* op1  = reinterpret_cast<Operation*> (1);
    Resource*  res1 = reinterpret_cast<Resource*> (2);
    Resource*  res2 = reinterpret_cast<Resource*> (3);
    Resource*  res3 = reinterpret_cast<Resource*> (4);

    GVK::RG::Pass p;
    p.AddInput (op1, res1);
    p.AddInput (op1, res2);
    p.AddOutput (op1, res3);

    EXPECT_EQ (1, p.GetAllOutputs ().size ());
    EXPECT_EQ (2, p.GetAllInputs ().size ());
    EXPECT_EQ (1, p.GetAllOperations ().size ());

    p.RemoveOutput (op1, res3);

    EXPECT_EQ (0, p.GetAllOutputs ().size ());
    EXPECT_EQ (2, p.GetAllInputs ().size ());
    EXPECT_EQ (1, p.GetAllOperations ().size ());

    p.RemoveInput (op1, res2);

    EXPECT_EQ (0, p.GetAllOutputs ().size ());
    EXPECT_EQ (1, p.GetAllInputs ().size ());
    EXPECT_EQ (1, p.GetAllOperations ().size ());

    p.RemoveInput (op1, res1);

    EXPECT_EQ (0, p.GetAllOutputs ().size ());
    EXPECT_EQ (0, p.GetAllInputs ().size ());
    EXPECT_EQ (0, p.GetAllOperations ().size ());
}


uint64_t Forrest_G (const uint64_t k, const uint64_t seed, const uint64_t g, const uint64_t m)
{
    uint64_t G = seed % m;
    uint64_t h = g;

    uint64_t i = (k + m) % m;

    while (i > 0) {
        if (i % 2 == 1) {
            G = (G * h) % m;
        }
        h = (h * h) % m;
        i = i / 2;
    }

    return G;
}


uint64_t Forrest_C (const uint64_t k, const uint64_t seed, const uint64_t g, const uint64_t c, const uint64_t m)
{
    uint64_t C = seed % m;
    uint64_t f = c;
    uint64_t h = g;
    uint64_t i = (k + m) % m;

    while (i > 0) {
        if (i % 2 == 1) {
            C = (C * h + f) % m;
        }
        f = (f * (h + 1)) % m;
        h = (h * h) % m;
        i = i / 2;
    }

    return C;
}

TEST_F (Empty, rng)
{
    double   sum     = 0.0;
    int      count   = 0;
    uint64_t modolus = 2147483647; //(static_cast<uint64_t> (1) << 31) - 1; // 2^31 - 1
    uint64_t mul_a   = 48271;      // minstd_rand
    uint64_t inc_c   = 0;

    for (int i = 10; i < 100000; ++i) {
        double val  = static_cast<double> (Forrest_G (i, 634, mul_a, modolus));
        double val2 = static_cast<double> (Forrest_C (i, 634, mul_a, 0, modolus));
        GVK_ASSERT (val == val2);
        const double perc = val / modolus;
        sum += perc;
        ++count;
    }

    std::cout << sum / count << std::endl;
}


TEST_F (HeadlessGoogleTestEnvironment, faszom)
{
    GVK::DeviceExtra& device = *env->deviceExtra;

    const char* vertSrc = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )";

    auto sp = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp->SetVertexShaderFromString (vertSrc);
    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (1, 0, 0, 0.5);
}
    )");

    auto sp2 = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp2->SetVertexShaderFromString (vertSrc);
    sp2->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (0, 0, 1, 0.5);
}
    )");

    GVK::Image2D renderTarget (*env->allocator,
                               GVK::Image2D::MemoryLocation::GPU,
                               512,
                               512,
                               VK_FORMAT_R8G8B8A8_SRGB,
                               VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                               1);

    GVK::ImageView2D renderTargetView (*env->device, renderTarget, 0, 1);
    GVK::ImageView2D renderTargetView2 (*env->device, renderTarget, 0, 1);

    GVK::DescriptorPool pool (device, 1024, 1024, 1024);
    GVK::DescriptorSetLayout setLayout (device, {});
    GVK::DescriptorSet set (device, pool, setLayout);

    VkAttachmentDescription attDesc1 = {};
    attDesc1.flags                   = 0;
    attDesc1.format                  = VK_FORMAT_R8G8B8A8_SRGB;
    attDesc1.samples                 = VK_SAMPLE_COUNT_1_BIT;
    attDesc1.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attDesc1.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    attDesc1.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attDesc1.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attDesc1.initialLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attDesc1.finalLayout             = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription attDesc2 = {};
    attDesc2.flags                   = 0;
    attDesc2.format                  = VK_FORMAT_R8G8B8A8_SRGB;
    attDesc2.samples                 = VK_SAMPLE_COUNT_1_BIT;
    attDesc2.loadOp                  = VK_ATTACHMENT_LOAD_OP_LOAD;
    attDesc2.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    attDesc2.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attDesc2.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attDesc2.initialLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attDesc2.finalLayout             = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference attRef1 = {};
    attRef1.attachment            = 0;
    attRef1.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference attRef2 = {};
    attRef2.attachment            = 0;
    attRef2.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    GVK::RG::ShaderPipeline::CompileSettings shaderPipelineSettings;
    shaderPipelineSettings.width                  = 512;
    shaderPipelineSettings.height                 = 512;
    shaderPipelineSettings.layout                 = setLayout.operator VkDescriptorSetLayout ();
    shaderPipelineSettings.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    shaderPipelineSettings.attachmentDescriptions = { attDesc1 };
    shaderPipelineSettings.attachmentReferences   = { attRef1 };
    shaderPipelineSettings.blendEnabled           = false;

    GVK::RG::ShaderPipeline::CompileSettings shaderPipelineSettings2;
    shaderPipelineSettings2.width                  = 512;
    shaderPipelineSettings2.height                 = 512;
    shaderPipelineSettings2.layout                 = setLayout.operator VkDescriptorSetLayout ();
    shaderPipelineSettings2.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    shaderPipelineSettings2.attachmentDescriptions = { attDesc2 };
    shaderPipelineSettings2.attachmentReferences   = { attRef2 };
    shaderPipelineSettings2.blendEnabled           = true;

    sp->Compile (std::move (shaderPipelineSettings));
    sp2->Compile (std::move (shaderPipelineSettings2));

    GVK::Framebuffer fb (*env->device, *sp->compileResult.renderPass, { renderTargetView.operator VkImageView () }, 512, 512);
    GVK::Framebuffer fb2 (*env->device, *sp2->compileResult.renderPass, { renderTargetView2.operator VkImageView () }, 512, 512);

    VkClearValue clearValue = {};
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
    
    VkImageMemoryBarrier transition            = {};
    transition.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    transition.pNext                           = nullptr;
    transition.srcAccessMask                   = flushAllMemory.srcAccessMask;
    transition.dstAccessMask                   = flushAllMemory.srcAccessMask;
    transition.oldLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition.newLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    transition.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    transition.image                           = renderTarget;
    transition.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    transition.subresourceRange.baseMipLevel   = 0;
    transition.subresourceRange.levelCount     = 1;
    transition.subresourceRange.baseArrayLayer = 0;
    transition.subresourceRange.layerCount     = 1;

    {
        GVK::CommandBuffer commandBuffer (*env->device, *env->commandPool);

        commandBuffer.Begin ();
        commandBuffer.Record<GVK::CommandPipelineBarrier> (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, std::vector<VkMemoryBarrier> { flushAllMemory }, std::vector<VkBufferMemoryBarrier> {}, std::vector<VkImageMemoryBarrier> { transition });
        commandBuffer.Record<GVK::CommandBeginRenderPass> (*sp->compileResult.renderPass, fb, VkRect2D { { 0, 0 }, { 512, 512 } }, std::vector<VkClearValue> { clearValue }, VK_SUBPASS_CONTENTS_INLINE);
        commandBuffer.Record<GVK::CommandBindPipeline> (VK_PIPELINE_BIND_POINT_GRAPHICS, *sp->compileResult.pipeline);
        commandBuffer.Record<GVK::CommandDraw> (6, 1, 0, 0);
        commandBuffer.Record<GVK::CommandEndRenderPass> ();
        commandBuffer.Record<GVK::CommandPipelineBarrier> (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, std::vector<VkMemoryBarrier> { flushAllMemory }, std::vector<VkBufferMemoryBarrier> {}, std::vector<VkImageMemoryBarrier> { transition });
        commandBuffer.Record<GVK::CommandBeginRenderPass> (*sp2->compileResult.renderPass, fb, VkRect2D { { 0, 0 }, { 512, 512 } }, std::vector<VkClearValue> { clearValue }, VK_SUBPASS_CONTENTS_INLINE);
        commandBuffer.Record<GVK::CommandBindPipeline> (VK_PIPELINE_BIND_POINT_GRAPHICS, *sp2->compileResult.pipeline);
        commandBuffer.Record<GVK::CommandDraw> (6, 1, 0, 0);
        commandBuffer.Record<GVK::CommandEndRenderPass> ();
        commandBuffer.Record<GVK::CommandPipelineBarrier> (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, std::vector<VkMemoryBarrier> { flushAllMemory }, std::vector<VkBufferMemoryBarrier> {}, std::vector<VkImageMemoryBarrier> { transition });
        commandBuffer.End ();

        commandBuffers.push_back (std::move (commandBuffer));
    }

    {
        GVK::SingleTimeCommand cmd (*env->device, *env->commandPool, *env->graphicsQueue);

        transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        transition.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        cmd.Record<GVK::CommandPipelineBarrier> (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                                 VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                                 std::vector<VkMemoryBarrier> { flushAllMemory },
                                                 std::vector<VkBufferMemoryBarrier> {},
                                                 std::vector<VkImageMemoryBarrier> { transition });
    }

    env->graphicsQueue->Submit ({}, {}, commandBuffers, {}, VK_NULL_HANDLE);
    env->graphicsQueue->Wait ();

    GVK::ImageData img (device, renderTarget, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    img.SaveTo (ReferenceImagesFolder / "killme.png");

    GVK::ImageData refimg (ReferenceImagesFolder / "pink.png");

    EXPECT_TRUE (refimg == img);
}


TEST_F (HeadlessGoogleTestEnvironment, faszom2)
{
    GVK::DeviceExtra& device = *env->deviceExtra;

    const char* vertSrc = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )";

    auto sp = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp->SetVertexShaderFromString (vertSrc);
    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (1, 0, 0, 0.5);
}
    )");

    auto sp2 = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp2->SetVertexShaderFromString (vertSrc);
    sp2->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (0, 0, 1, 0.5);
}
    )");

    std::shared_ptr<GVK::RG::WritableImageResource> renderTarget = std::make_shared<GVK::RG::WritableImageResource> (VK_FILTER_LINEAR, 512, 512, 1, VK_FORMAT_R8G8B8A8_SRGB);

    GVK::RG::GraphSettings s;
    s.framesInFlight = 1;
    s.device         = env->deviceExtra.get ();

    renderTarget->Compile (s);

    std::shared_ptr<GVK::RG::RenderOperation> renderOp  = std::make_shared<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 6), std::move (sp));
    std::shared_ptr<GVK::RG::RenderOperation> renderOp2 = std::make_shared<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 6), std::move (sp2));

    s.connectionSet.Add (renderOp, renderTarget,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   [] () -> VkFormat { return VK_FORMAT_R8G8B8A8_SRGB; },
                                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                   1,
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (renderOp2, renderTarget,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   [] () -> VkFormat { return VK_FORMAT_R8G8B8A8_SRGB; },
                                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                   1,
                                                                   VK_ATTACHMENT_LOAD_OP_LOAD,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));


    renderOp->Compile (s, 512, 512);
    renderOp2->Compile (s, 512, 512);

    VkClearValue clearValue = {};
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
    
    VkImageMemoryBarrier transition            = {};
    transition.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    transition.pNext                           = nullptr;
    transition.srcAccessMask                   = flushAllMemory.srcAccessMask;
    transition.dstAccessMask                   = flushAllMemory.srcAccessMask;
    transition.oldLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition.newLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    transition.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    transition.image                           = *renderTarget->images[0]->image;
    transition.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    transition.subresourceRange.baseMipLevel   = 0;
    transition.subresourceRange.levelCount     = 1;
    transition.subresourceRange.baseArrayLayer = 0;
    transition.subresourceRange.layerCount     = 1;

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
        commandBuffer.Record<GVK::CommandPipelineBarrier> (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, std::vector<VkMemoryBarrier> { flushAllMemory }, std::vector<VkBufferMemoryBarrier> {}, std::vector<VkImageMemoryBarrier> { transition });
        commandBuffer.End ();

        commandBuffers.push_back (std::move (commandBuffer));
    }

    {
        GVK::SingleTimeCommand cmd (*env->device, *env->commandPool, *env->graphicsQueue);

        transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        transition.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        cmd.Record<GVK::CommandPipelineBarrier> (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                                 VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                                 std::vector<VkMemoryBarrier> { flushAllMemory },
                                                 std::vector<VkBufferMemoryBarrier> {},
                                                 std::vector<VkImageMemoryBarrier> { transition });
    }

    env->graphicsQueue->Submit ({}, {}, commandBuffers, {}, VK_NULL_HANDLE);
    env->graphicsQueue->Wait ();

    GVK::ImageData img (device, *renderTarget->images[0]->image, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    img.SaveTo (ReferenceImagesFolder / "killme.png");

    GVK::ImageData refimg (ReferenceImagesFolder / "pink.png");

    EXPECT_TRUE (refimg == img);
}


TEST_F (HeadlessGoogleTestEnvironment, faszom3)
{
    GVK::DeviceExtra& device = *env->deviceExtra;

    const char* vertSrc = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )";

    auto sp = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp->SetVertexShaderFromString (vertSrc);
    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (1, 0, 0, 0.5);
}
    )");

    auto sp2 = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp2->SetVertexShaderFromString (vertSrc);
    sp2->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (0, 0, 1, 0.5);
}
    )");

    std::shared_ptr<GVK::RG::WritableImageResource> renderTarget = std::make_shared<GVK::RG::WritableImageResource> (VK_FILTER_LINEAR, 512, 512, 1, VK_FORMAT_R8G8B8A8_SRGB);

    std::shared_ptr<GVK::RG::RenderOperation> renderOp  = std::make_shared<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 6), std::move (sp));
    std::shared_ptr<GVK::RG::RenderOperation> renderOp2 = std::make_shared<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 6), std::move (sp2));

    GVK::RG::GraphSettings s;

    s.framesInFlight = 1;
    s.device         = env->deviceExtra.get ();

    s.connectionSet.Add (renderOp, renderTarget,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   renderTarget->GetFormatProvider (),
                                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                   renderTarget->GetFinalLayout (),
                                                                   1,
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (renderOp2, renderTarget,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   renderTarget->GetFormatProvider (),
                                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                   renderTarget->GetFinalLayout (),
                                                                   1,
                                                                   VK_ATTACHMENT_LOAD_OP_LOAD,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    

    GVK::RG::RenderGraph rg;

    rg.Compile (std::move (s));

    VkClearValue clearValue = {};
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
    
    VkImageMemoryBarrier transition            = {};
    transition.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    transition.pNext                           = nullptr;
    transition.srcAccessMask                   = flushAllMemory.srcAccessMask;
    transition.dstAccessMask                   = flushAllMemory.srcAccessMask;
    transition.oldLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition.newLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    transition.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    transition.image                           = *renderTarget->images[0]->image;
    transition.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    transition.subresourceRange.baseMipLevel   = 0;
    transition.subresourceRange.levelCount     = 1;
    transition.subresourceRange.baseArrayLayer = 0;
    transition.subresourceRange.layerCount     = 1;

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
        commandBuffer.Record<GVK::CommandPipelineBarrier> (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, std::vector<VkMemoryBarrier> { flushAllMemory }, std::vector<VkBufferMemoryBarrier> {}, std::vector<VkImageMemoryBarrier> { });
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

    GVK::ImageData img (device, *renderTarget->images[0]->image, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    img.SaveTo (ReferenceImagesFolder / "killme.png");

    GVK::ImageData refimg (ReferenceImagesFolder / "pink.png");

    EXPECT_TRUE (refimg == img);
}


TEST_F (HeadlessGoogleTestEnvironment, faszom4)
{
    GVK::DeviceExtra& device = *env->deviceExtra;

    const char* vertSrc = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )";

    auto sp = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp->SetVertexShaderFromString (vertSrc);
    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (1, 0, 0, 0.5);
}
    )");

    auto sp2 = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp2->SetVertexShaderFromString (vertSrc);
    sp2->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (0, 0, 1, 0.5);
}
    )");

    std::shared_ptr<GVK::RG::WritableImageResource> renderTarget = std::make_shared<GVK::RG::WritableImageResource> (VK_FILTER_LINEAR, 512, 512, 1, VK_FORMAT_R8G8B8A8_SRGB);

    std::shared_ptr<GVK::RG::RenderOperation> renderOp  = std::make_shared<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 6), std::move (sp));
    std::shared_ptr<GVK::RG::RenderOperation> renderOp2 = std::make_shared<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 6), std::move (sp2));

    GVK::RG::GraphSettings s;

    s.framesInFlight = 1;
    s.device         = env->deviceExtra.get ();

    s.connectionSet.Add (renderOp, renderTarget,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   [] () -> VkFormat { return VK_FORMAT_R8G8B8A8_SRGB; },
                                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                   1,
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (renderOp2, renderTarget,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   [] () -> VkFormat { return VK_FORMAT_R8G8B8A8_SRGB; },
                                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                   1,
                                                                   VK_ATTACHMENT_LOAD_OP_LOAD,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    

    GVK::RG::RenderGraph rg;

    rg.Compile (std::move (s));

    env->graphicsQueue->Submit ({}, {}, { &rg.commandBuffers[0] }, {}, VK_NULL_HANDLE);
    env->graphicsQueue->Wait ();

    GVK::ImageData img (device, *renderTarget->images[0]->image, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    img.SaveTo (ReferenceImagesFolder / "killme.png");

    GVK::ImageData refimg (ReferenceImagesFolder / "pink.png");

    EXPECT_TRUE (refimg == img);
}



TEST_F (HeadlessGoogleTestEnvironment, DISABLED_LCGShader)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();

    GVK::RG::RenderGraph graph;

    auto sp = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )");

    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable

layout (location = 0) in vec2 textureCoords;
layout (location = 0) out vec4 outColor;

uint64_t Forrest_G (const uint64_t k, const uint64_t seed, const uint64_t g, const uint64_t m)
{
    uint64_t G = seed % m;
    uint64_t h = g;

    uint64_t i = (k + m) % m;

    while (i > 0) {
        if (i % 2 == 1) {
            G = (G * h) % m;
        }
        h = (h * h) % m;
        i = i / 2;
    }

    return G;
}

uint64_t Forrest_C (const uint64_t k, const uint64_t seed, const uint64_t g, const uint64_t c, const uint64_t m)
{
    uint64_t C = seed % m;
    uint64_t f = c;
    uint64_t h = g;
    uint64_t i = (k + m) % m;

    while (i > 0) {
        if (i % 2 == 1) {
            C = (C * h + f) % m;
        }
        f = (f * (h + 1)) % m;
        h = (h * h) % m;
        i = i / 2;
    }

    return C;
}

void main ()
{
    float rng = float (Forrest_C (int (textureCoords.y * 800 * 600 + textureCoords.x * 600), 456, 48271, 0, 2147483647)) / float(2147483647);
 
   outColor = vec4 (vec3 (rng), 1);
}
    )");


    std::shared_ptr<GVK::RG::RenderOperation> redFillOperation = std::make_unique<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 6), std::move (sp));

    std::shared_ptr<GVK::RG::ImageResource> red = std::make_unique<GVK::RG::WritableImageResource> (512, 512);

    GVK::RG::GraphSettings s (device, 3);
    s.connectionSet.Add (redFillOperation, red,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   red->GetFormatProvider (),
                                                                   red->GetFinalLayout (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    graph.Compile (std::move (s));

    graph.Submit (0);

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    CompareImages ("lcg", *red->GetImages ()[0], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HeadlessGoogleTestEnvironment, Spirvrross2)
{
    std::unique_ptr<GVK::ShaderModule> sm = GVK::ShaderModule::CreateFromGLSLString (GetDevice (), GVK::ShaderKind::Fragment, R"(#version 450

struct A {
    vec3 abc;
    double hello;
};

struct B {
    A bs[3];
};

struct C {
    B cs[3];
    mat3x4 WTF;
    float hehe[3];
};

layout (std140, binding = 2) uniform Quadrics {
    float dddddddddddd;
    vec3 dddddddddddd33;
    C quadrics[2];
    C theotherone[9];
};

layout (location = 0) out vec4 presented;
layout (binding = 3) uniform sampler3D agySampler;
layout (binding = 4) uniform sampler2D matcapSampler;

void main ()
{
    presented = vec4 (vec3 (1), dddddddddddd);
}

)");

    GVK::SR::ShaderUData refl (sm);

    refl["Quadrics"]["quadrics"][0]["WTF"] = glm::mat3x4 ();


    EXPECT_EQ (4256, refl.GetUbo ("Quadrics")->GetFullSize ());
}


TEST_F (HeadlessGoogleTestEnvironment, CompileTest)
{
    GVK::DeviceExtra& device = GetDeviceExtra ();

    GVK::RG::RenderOperation op (std::make_unique<GVK::DrawRecordableInfo> (1, 3),
                                 std::make_unique<GVK::RG::ShaderPipeline> (device, std::vector<std::filesystem::path> {
                                                                                        ShadersFolder / "test.vert",
                                                                                        ShadersFolder / "test.frag",
                                                                                    }));
}


TEST_F (HeadlessGoogleTestEnvironment, ShaderCompileTests)
{
    try {
        GVK::ShaderModule::CreateFromGLSLString (GetDevice (), GVK::ShaderKind::Vertex, R"(
        #version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

    vec2 uvs[6] = vec2[] (
        vec2 (0.f, 0.f),
        vec2 (0.f, 1.f),
        vec2 (1.f,d 1.f),
        vec2 (1.f, 1.f),
        vec2 (0.f, 0.f),
        vec2 (1.f, 0.f)
    );

    vec2 positions[6] = vec2[] (
        vec2 (-1.f, -1.f),
        vec2 (-1.f, +1.f),
        vec2 (+1.f, +1.f),
        vec2 (+1.f, +1.f),
        vec2 (-1.f, -1.f),
        vec2 (+1.f, -1.f)
    );


    void main ()
    {
        gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
        textureCoords = uvs[gl_VertexIndex];
    }
)");

        FAIL () << "expected ShaderCompileException";

    } catch (GVK::ShaderCompileException&) {
        // good
    } catch (...) {
        FAIL () << "expected ShaderCompileException";
    }
}


TEST_F (HeadlessGoogleTestEnvironment, ImageMap_TextureArray)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();

    GVK::RG::GraphSettings s (device, 3);
    GVK::RG::RenderGraph   graph;

    auto sp = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )");

    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 7) uniform sampler2D textureArray_RGBA_32x32[2];
layout (binding = 8) uniform sampler2D textureArray_R_32x32[2];

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (
        texture (textureArray_RGBA_32x32[0], vec2 ( 0.0/32.0,  0.0/32.0)).r,
        texture (textureArray_RGBA_32x32[0], vec2 (15.0/32.0, 28.0/32.0)).g,
        texture (textureArray_R_32x32[1],    vec2 ( 7.0/32.0, 31.0/32.0)).r,
        1
    );
}
    )");

    std::shared_ptr<GVK::RG::RenderOperation> redFillOperation = std::make_unique<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 6), std::move (sp));

    s.connectionSet.Add (redFillOperation);

    GVK::RG::ImageMap imgMap = GVK::RG::CreateEmptyImageResources (s.connectionSet, [&] (const GVK::SR::Sampler& sampler) -> std::optional<GVK::RG::CreateParams> {
        if (sampler.name == "textureArray_RGBA_32x32") {
            return std::make_tuple (glm::uvec3 { 32, 32, 0 }, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FILTER_NEAREST);
        }
        if (sampler.name == "textureArray_R_32x32") {
            return std::make_tuple (glm::uvec3 { 32, 32, 0 }, VK_FORMAT_R32_SFLOAT, VK_FILTER_NEAREST);
        }
        return std::nullopt;
    });

    std::shared_ptr<GVK::RG::WritableImageResource> presented = std::make_unique<GVK::RG::WritableImageResource> (512, 512);

    s.connectionSet.Add (redFillOperation, presented,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   presented->GetFormatProvider (),
                                                                   presented->GetFinalLayout (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    graph.Compile (std::move (s));

    {
        std::shared_ptr<GVK::RG::ReadOnlyImageResource> textureArray = imgMap.FindByName ("textureArray_R_32x32");
        GVK_ASSERT (textureArray != nullptr);
        std::vector<float> pixelData (32 * 32);
        std::generate_n (pixelData.begin (), 32 * 32, [] () {
            return 0.3f;
        });
        textureArray->CopyTransitionTransfer (pixelData);
    }
    {
        std::shared_ptr<GVK::RG::ReadOnlyImageResource> textureArray = imgMap.FindByName ("textureArray_RGBA_32x32");
        GVK_ASSERT (textureArray != nullptr);
        std::vector<glm::vec4> pixelData (32 * 32);
        std::generate_n (pixelData.begin (), 32 * 32, [] () {
            return glm::vec4 (0.7, 0.6, 0.8, 1.0);
        });
        textureArray->CopyTransitionTransfer (pixelData);
    }

    graph.Submit (0);

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    CompareImages ("textureArray", *presented->GetImages ()[0], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

TEST_F (HeadlessGoogleTestEnvironment, RenderRedImage)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();

    GVK::RG::GraphSettings s (device, 3);
    GVK::RG::RenderGraph   graph;

    auto sp = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )");

    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (1, 0, 0, 1);
}
    )");

    std::shared_ptr<GVK::RG::RenderOperation> redFillOperation = std::make_unique<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 6), std::move (sp));

    std::shared_ptr<GVK::RG::ImageResource> red = std::make_unique<GVK::RG::WritableImageResource> (512, 512);

    s.connectionSet.Add (redFillOperation, red,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   red->GetFormatProvider (),
                                                                   red->GetFinalLayout (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    graph.Compile (std::move (s));

    graph.Submit (0);
    graph.Submit (1);
    graph.Submit (2);

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    CompareImages ("red", *red->GetImages ()[0], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    CompareImages ("red", *red->GetImages ()[1], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    CompareImages ("red", *red->GetImages ()[2], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HeadlessGoogleTestEnvironment, TransferOperation)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();

    GVK::RG::GraphSettings s (device, 1);
    GVK::RG::RenderGraph   graph;

    auto sp = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )");

    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (1, 0, 0, 1);
}
    )");

    std::shared_ptr<GVK::RG::RenderOperation> redFillOperation = std::make_unique<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 6), std::move (sp));

    std::shared_ptr<GVK::RG::TransferOperation> transfer = std::make_unique<GVK::RG::TransferOperation> ();

    std::shared_ptr<GVK::RG::WritableImageResource> red       = std::make_unique<GVK::RG::WritableImageResource> (512, 512);
    std::shared_ptr<GVK::RG::WritableImageResource> duplicate = std::make_unique<GVK::RG::WritableImageResource> (512, 512);


    s.connectionSet.Add (redFillOperation, red,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   red->GetFormatProvider (),
                                                                   red->GetFinalLayout (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));


    s.connectionSet.Add (red, transfer,
                         std::make_unique<GVK::RG::ImageInputBinding> (0, *red));

    s.connectionSet.Add (transfer, duplicate,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   duplicate->GetFormatProvider (),
                                                                   duplicate->GetFinalLayout (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));


    graph.Compile (std::move (s));

    graph.Submit (0);

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    CompareImages ("red", *duplicate->GetImages ()[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}


TEST_F (HeadlessGoogleTestEnvironment, RenderGraphUseTest)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();

    GVK::RG::GraphSettings s (device, 4);
    GVK::RG::RenderGraph   graph;

    std::shared_ptr<GVK::RG::WritableImageResource> presented = std::make_unique<GVK::RG::WritableImageResource> (512, 512);
    std::shared_ptr<GVK::RG::WritableImageResource> green     = std::make_unique<GVK::RG::WritableImageResource> (512, 512);
    std::shared_ptr<GVK::RG::WritableImageResource> red       = std::make_unique<GVK::RG::WritableImageResource> (512, 512);
    std::shared_ptr<GVK::RG::WritableImageResource> finalImg  = std::make_unique<GVK::RG::WritableImageResource> (512, 512);


    /*
        green -> dummyPass -> presented -> secondPass -> finalImg
                           -> red
    */

    std::shared_ptr<GVK::RG::RenderOperation> dummyPass = std::make_unique<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 3),
                                                                                                      std::make_unique<GVK::RG::ShaderPipeline> (device, std::vector<std::filesystem::path> {
                                                                                                                                                             ShadersFolder / "test.vert",
                                                                                                                                                             ShadersFolder / "test.frag",
                                                                                                                                                         }));

    std::shared_ptr<GVK::RG::RenderOperation> secondPass = std::make_unique<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 3),
                                                                                                       std::make_unique<GVK::RG::ShaderPipeline> (device, std::vector<std::filesystem::path> {
                                                                                                                                                              ShadersFolder / "fullscreenquad.vert",
                                                                                                                                                              ShadersFolder / "fullscreenquad.frag",
                                                                                                                                                          }));


    s.connectionSet.Add (green, dummyPass, std::make_unique<GVK::RG::ImageInputBinding> (0, *green));
    s.connectionSet.Add (red, secondPass, std::make_unique<GVK::RG::ImageInputBinding> (0, *red));

    s.connectionSet.Add (dummyPass, presented,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   presented->GetFormatProvider (),
                                                                   presented->GetFinalLayout (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (dummyPass, red,
                         std::make_unique<GVK::RG::OutputBinding> (1,
                                                                   red->GetFormatProvider (),
                                                                   red->GetFinalLayout (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));


    s.connectionSet.Add (secondPass, finalImg,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   finalImg->GetFormatProvider (),
                                                                   finalImg->GetFinalLayout (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    graph.Compile (std::move (s));

    for (uint32_t i = 0; i < 1; ++i) {
        graph.Submit (i);
    }

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    GVK::ImageData (device, *green->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).SaveTo (ReferenceImagesFolder / "green.png");
    GVK::ImageData (device, *presented->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).SaveTo (ReferenceImagesFolder / "presented.png");
    GVK::ImageData (device, *red->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).SaveTo (ReferenceImagesFolder / "red.png");
    GVK::ImageData (device, *finalImg->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).SaveTo (ReferenceImagesFolder / "final.png");

    ASSERT_TRUE (GVK::ImageData (device, *presented->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) == GVK::ImageData (ReferenceImagesFolder / "black.png"));
}


TEST_F (HeadlessGoogleTestEnvironment, RenderGraphUseTest_TwoOperationsRenderingToOutput)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();

    GVK::RG::GraphSettings s (device, 1);
    GVK::RG::RenderGraph   graph;

    std::shared_ptr<GVK::RG::WritableImageResource> presented = std::make_unique<GVK::RG::WritableImageResource> (VK_FILTER_LINEAR, 512, 512, 1, VK_FORMAT_R8G8B8A8_SRGB);


    /*
        firstPass  ---> presented
        secondPass  _/ 
    */

    // will turn into

    /*
        firstPass ---> presented ---> secondPass ---> presented
    */
    
    const char* vertSrc = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )";

    auto sp = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp->SetVertexShaderFromString (vertSrc);
    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (1, 0, 0, 0.5);
}
    )");

    auto sp2 = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp2->SetVertexShaderFromString (vertSrc);
    sp2->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (0, 0, 1, 0.5);
}
    )");

    std::shared_ptr<GVK::RG::RenderOperation> firstPass  = std::make_unique<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 6), std::move (sp));
    firstPass->SetName ("FIRST");
    firstPass->compileSettings.blendEnabled = false;

    std::shared_ptr<GVK::RG::RenderOperation> secondPass = std::make_unique<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 6), std::move (sp2));
    secondPass->SetName ("SECOND");

    secondPass->compileSettings.blendEnabled = true;

    s.connectionSet.Add (firstPass, presented,
                       std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   presented->GetFormatProvider (),
                                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                   presented->GetFinalLayout (),
                                                                   1,
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (secondPass, presented,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   presented->GetFormatProvider (),
                                                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                   presented->GetFinalLayout (),
                                                                   1,
                                                                   VK_ATTACHMENT_LOAD_OP_LOAD,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));


    graph.Compile (std::move (s));

    ASSERT_EQ (2, graph.GetPassCount ());

    GVK::ImageData referenceImage (ReferenceImagesFolder / "pink.png");

    const size_t renderCount = 1;
    size_t matchCount = 0;

    for (size_t i = 0; i < renderCount; ++i) {
        std::this_thread::sleep_for (std::chrono::milliseconds (200));

        graph.Submit (0);

        vkQueueWaitIdle (graphicsQueue);
        vkDeviceWaitIdle (device);

        GVK::ImageData (device, *presented->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).SaveTo (ReferenceImagesFolder / "presentedTwo.png");
 
        if (GVK::ImageData (device, *presented->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) == referenceImage) {
            ++matchCount;
        }
    }

    EXPECT_EQ (renderCount, matchCount);

}


TEST_F (HiddenWindowGoogleTestEnvironment, SwapchainTest)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();
    GVK::Swapchain&   swapchain     = GetSwapchain ();


    GVK::RG::GraphSettings s (device, swapchain.GetImageCount ());
    GVK::RG::RenderGraph   graph;

    auto sp = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )");

    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outCopy;

void main () {
    vec4 result = vec4 (uv, 0, 1);
    outColor = result;
    outCopy = result;
}
    )");

    std::shared_ptr<GVK::RG::RenderOperation> redFillOperation = std::make_unique<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, 6), std::move (sp));

    std::shared_ptr<GVK::RG::ImageResource> presentedCopy = std::make_unique<GVK::RG::WritableImageResource> (800, 600);
    std::shared_ptr<GVK::RG::ImageResource> presented     = std::make_unique<GVK::RG::SwapchainImageResource> (*presentable);

    s.connectionSet.Add (redFillOperation, presented,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   presented->GetFormatProvider (),
                                                                   presented->GetFinalLayout (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (redFillOperation, presentedCopy,
                         std::make_unique<GVK::RG::OutputBinding> (1,
                                                                   presentedCopy->GetFormatProvider (),
                                                                   presentedCopy->GetFinalLayout (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));
    graph.Compile (std::move (s));

    GVK::RG::BlockingGraphRenderer renderer (device, swapchain);
    uint32_t count = 0;
    auto cb = renderer.GetConditionalDrawCallback ([&] () -> GVK::RG::RenderGraph& { return graph; }, [&] () -> bool {
        return ++count > 10;
    });
    window->DoEventLoop (cb);
}


TEST_F (HiddenWindowGoogleTestEnvironment, VertexAndIndexBufferTest)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();
    GVK::Swapchain&   swapchain     = GetSwapchain ();

    GVK::RG::GraphSettings s (device, swapchain.GetImageCount ());
    GVK::RG::RenderGraph   graph;

    auto sp = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in float asd;

layout (location = 0) out vec2 textureCoords;
layout (location = 1) out float asdout;


void main() {
    gl_Position = vec4 (position, 0.0, 1.0);
    textureCoords = uv;
    asdout = asd;
}
    )");

    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 1) in float asdout;
layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outCopy;

void main () {
    vec4 result = vec4 (vec3 (uv, 0.f), 1);
    outColor = result;
    outCopy = result;
}
    )");

    struct Vert {
        glm::vec2 position;
        glm::vec2 uv;
        float     asd;
    };

    GVK::VertexBufferTransferable<Vert> vbb (device, 4, { GVK::ShaderTypes::Vec2f, GVK::ShaderTypes::Vec2f, GVK::ShaderTypes::Float }, VK_VERTEX_INPUT_RATE_VERTEX);
    vbb = std::vector<Vert> {
        { glm::vec2 (-1.f, -1.f), glm::vec2 (0.f, 0.f), 0.1f },
        { glm::vec2 (-1.f, +1.f), glm::vec2 (0.f, 1.f), 0.2f },
        { glm::vec2 (+1.f, +1.f), glm::vec2 (1.f, 1.f), 0.3f },
        { glm::vec2 (+1.f, -1.f), glm::vec2 (1.f, 0.f), 0.6f },
    };
    vbb.Flush ();

    GVK::IndexBufferTransferable ib (device, 6);
    ib.data = { 0, 1, 2, 0, 3, 2 };
    ib.Flush ();

    std::shared_ptr<GVK::RG::RenderOperation> redFillOperation = std::make_unique<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, static_cast<uint32_t> (vbb.data.size ()), vbb.buffer.GetBufferToBind (), vbb.info.bindings, vbb.info.attributes, static_cast<uint32_t> (ib.data.size ()), ib.buffer.GetBufferToBind ()),
                                                                                                             std::move (sp));

    std::shared_ptr<GVK::RG::WritableImageResource>  presentedCopy = std::make_unique<GVK::RG::WritableImageResource> (800, 600);
    std::shared_ptr<GVK::RG::SwapchainImageResource> presented     = std::make_unique<GVK::RG::SwapchainImageResource> (*presentable);


    s.connectionSet.Add (redFillOperation, presented,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   presented->GetFormatProvider (),
                                                                   presented->GetFinalLayout (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (redFillOperation, presentedCopy,
                         std::make_unique<GVK::RG::OutputBinding> (1,
                                                                   presentedCopy->GetFormatProvider (),
                                                                   presentedCopy->GetFinalLayout (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    graph.Compile (std::move (s));

    GVK::RG::BlockingGraphRenderer renderer (device, swapchain);
    uint32_t count = 0;
    auto cb = renderer.GetConditionalDrawCallback ([&] () -> GVK::RG::RenderGraph& { return graph; }, [&] () -> bool {
      return ++count > 10;
    });
    window->DoEventLoop (cb);

    CompareImages ("uv", *presentedCopy->images[0]->image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HiddenWindowGoogleTestEnvironment, BasicUniformBufferTest)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();
    GVK::Swapchain&   swapchain     = GetSwapchain ();

    GVK::RG::GraphSettings s (device, swapchain.GetImageCount ());
    GVK::RG::RenderGraph   graph;

    auto sp = std::make_unique<GVK::RG::ShaderPipeline> (device);
    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform Time {
    float time;
} time;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in float asd;

layout (location = 0) out vec2 textureCoords;
layout (location = 1) out float asdout;


void main() {
    gl_Position = vec4 (position + vec2 (time.time), 0.0, 1.0);
    textureCoords = uv;
    asdout = asd;
}
    )");

    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 1) in float asdout;
layout (location = 0) in vec2 uv;

layout (location = 2) out vec4 presented;
layout (location = 0) out vec4 copy[2];

void main () {
    vec4 result = vec4 (vec3 (uv, 0.f), 1);
    presented = result;
    copy[0] = result;
    copy[1] = result;
}
    )");

    struct Vert {
        glm::vec2 position;
        glm::vec2 uv;
        float     asd;
    };

    std::shared_ptr<GVK::VertexBufferTransferable<Vert>> vbb = std::make_unique<GVK::VertexBufferTransferable<Vert>> (
        device, 4, std::vector<VkFormat> { GVK::ShaderTypes::Vec2f, GVK::ShaderTypes::Vec2f, GVK::ShaderTypes::Float }, VK_VERTEX_INPUT_RATE_VERTEX);

    *vbb = std::vector<Vert> {
        { glm::vec2 (-1.f, -1.f), glm::vec2 (0.f, 0.f), 0.1f },
        { glm::vec2 (-1.f, +1.f), glm::vec2 (0.f, 1.f), 0.2f },
        { glm::vec2 (+1.f, +1.f), glm::vec2 (1.f, 1.f), 0.3f },
        { glm::vec2 (+1.f, -1.f), glm::vec2 (1.f, 0.f), 0.6f },
    };
    vbb->Flush ();

    GVK::IndexBufferTransferable ib (device, 6);
    ib.data = { 0, 1, 2, 0, 3, 2 };
    ib.Flush ();

    std::shared_ptr<GVK::RG::RenderOperation> redFillOperation = std::make_unique<GVK::RG::RenderOperation> (std::make_unique<GVK::DrawRecordableInfo> (1, *vbb, ib), std::move (sp));

    std::shared_ptr<GVK::RG::SwapchainImageResource> presented     = std::make_unique<GVK::RG::SwapchainImageResource> (*presentable);
    std::shared_ptr<GVK::RG::WritableImageResource>  presentedCopy = std::make_unique<GVK::RG::WritableImageResource> (VK_FILTER_LINEAR, 800, 600, 2);
    std::shared_ptr<GVK::RG::CPUBufferResource>      unif          = std::make_unique<GVK::RG::CPUBufferResource> (4);


    s.connectionSet.Add (unif, redFillOperation,
                         std::make_unique<GVK::RG::UniformInputBinding> (0, *unif));

    s.connectionSet.Add (redFillOperation, presented,
                         std::make_unique<GVK::RG::OutputBinding> (2,
                                                                   presented->GetFormatProvider (),
                                                                   presented->GetFinalLayout (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));


    s.connectionSet.Add (redFillOperation, presentedCopy,
                         std::make_unique<GVK::RG::OutputBinding> (0,
                                                                   presentedCopy->GetFormatProvider (),
                                                                   presentedCopy->GetFinalLayout (),
                                                                   presentedCopy->GetLayerCount (),
                                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                   VK_ATTACHMENT_STORE_OP_STORE));

    graph.Compile (std::move (s));

    GVK::RG::BlockingGraphRenderer renderer (device, swapchain);

    GVK::EventObserver obs;
    obs.Observe (renderer.preSubmitEvent, [&] (GVK::RG::RenderGraph&, uint32_t frameIndex, uint64_t) {
        float time = 0.5f;
        unif->GetMapping (frameIndex).Copy (time);
    });

    uint32_t count = 0;
    auto cb = renderer.GetConditionalDrawCallback ([&] () -> GVK::RG::RenderGraph& { return graph; }, [&] () -> bool {
      return ++count > 10;
    });
    window->DoEventLoop (cb);

    CompareImages ("uvoffset", *presentedCopy->images[0]->image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}
