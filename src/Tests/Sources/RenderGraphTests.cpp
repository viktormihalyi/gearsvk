#include "TestEnvironment.hpp"

#include "RenderGraph/DrawRecordable/FullscreenQuad.hpp"
#include "RenderGraph/GraphRenderer.hpp"
#include "RenderGraph/GraphSettings.hpp"
#include "RenderGraph/Operation.hpp"
#include "RenderGraph/RenderGraph.hpp"
#include "RenderGraph/Resource.hpp"
#include "RenderGraph/ShaderPipeline.hpp"
#include "RenderGraph/UniformReflection.hpp"
#include "RenderGraph/UniformView.hpp"
#include "RenderGraph/VulkanEnvironment.hpp"
#include "RenderGraph/Window/GLFWWindow.hpp"

#include "Utils/SourceLocation.hpp"
#include "Utils/Timer.hpp"
#include "Utils/Utils.hpp"

#include "VulkanWrapper/Allocator.hpp"
#include "VulkanWrapper/DeviceExtra.hpp"
#include "VulkanWrapper/Utils/ImageData.hpp"
#include "VulkanWrapper/Utils/VulkanUtils.hpp"
#include "VulkanWrapper/VulkanWrapper.hpp"
#include "VulkanWrapper/Commands.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>


const std::filesystem::path ShadersFolder = std::filesystem::current_path () / "TestData" / "shaders";



TEST_F (HeadlessTestEnvironment, DISABLED_LCGShader)
{
    const std::string fragSrc = R"(
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
    )";

    std::shared_ptr<RG::RenderOperation> redFillOperation = RG::RenderOperation::Builder (GetDevice ())
                                                                .SetVertices (std::make_unique<RG::DrawRecordableInfo> (1, 6))
                                                                .SetPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                                                                .SetVertexShader (passThroughVertexShader)
                                                                .SetFragmentShader (fragSrc)
                                                                .Build ();

    std::shared_ptr<RG::WritableImageResource> red = std::make_unique<RG::WritableImageResource> (512, 512);

    RG::GraphSettings s (GetDeviceExtra (), 3);

    auto& aTable2 = redFillOperation->compileSettings.attachmentProvider;
    aTable2->table.push_back ({ "outColor", GVK::ShaderKind::Fragment, { red->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, red->GetImageViewForFrameProvider (), red->GetInitialLayout (), red->GetFinalLayout () } });

    s.connectionSet.Add (redFillOperation, red);

    RG::RenderGraph graph;
    graph.Compile (std::move (s));
    graph.Submit (0);

    env->Wait ();

    CompareImages ("lcg", *red->GetImages ()[0], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HeadlessTestEnvironment, UniformReflection_NestedTypes)
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

    SR::ReflCompiler reflCompiler (sm->GetBinary ());
    auto                  ubos = SR::GetUBOsFromBinary (reflCompiler);
    SR::ShaderUData  refl (ubos);

    refl["Quadrics"]["quadrics"][0]["WTF"] = glm::mat3x4 ();


    EXPECT_EQ (4256, refl.GetUbo ("Quadrics")->GetFullSize ());
}


TEST_F (HeadlessTestEnvironment, ShaderPipeline_CompileTest)
{
    GVK::DeviceExtra& device = GetDeviceExtra ();

    RG::RenderOperation op (std::make_unique<RG::DrawRecordableInfo> (1, 3),
                            std::make_unique<RG::ShaderPipeline> (device, std::vector<std::filesystem::path> {
                                                                              ShadersFolder / "test.vert",
                                                                              ShadersFolder / "test.frag",
                                                                          }));
}


TEST_F (HeadlessTestEnvironment, ShaderModule_CompileError)
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


TEST_F (HeadlessTestEnvironment, UniformReflection_ImageMap_TextureArray)
{
    const std::string fragSrc = R"(
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
    )";

    std::shared_ptr<RG::RenderOperation> redFillOperation = RG::RenderOperation::Builder (GetDevice ())
                                                                .SetVertices (std::make_unique<RG::DrawRecordableInfo> (1, 6))
                                                                .SetPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                                                                .SetVertexShader (passThroughVertexShader)
                                                                .SetFragmentShader (fragSrc)
                                                                .Build ();

    RG::GraphSettings s (GetDeviceExtra (), 3);

    s.connectionSet.Add (redFillOperation);

    RG::ImageMap imgMap = RG::CreateEmptyImageResources (s.connectionSet, [&] (const SR::Sampler& sampler) -> std::optional<RG::CreateParams> {
        if (sampler.name == "textureArray_RGBA_32x32") {
            return std::make_tuple (glm::uvec3 { 32, 32, 0 }, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FILTER_NEAREST);
        }
        if (sampler.name == "textureArray_R_32x32") {
            return std::make_tuple (glm::uvec3 { 32, 32, 0 }, VK_FORMAT_R32_SFLOAT, VK_FILTER_NEAREST);
        }
        return std::nullopt;
    });

    std::shared_ptr<RG::WritableImageResource> presented = std::make_unique<RG::WritableImageResource> (512, 512);

    auto& aTable2 = redFillOperation->compileSettings.attachmentProvider;
    aTable2->table.push_back ({ "outColor", GVK::ShaderKind::Fragment, { presented->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, presented->GetImageViewForFrameProvider (), presented->GetInitialLayout (), presented->GetFinalLayout () } });

    s.connectionSet.Add (redFillOperation, presented);

    RG::RenderGraph graph;
    graph.Compile (std::move (s));
    {
        std::shared_ptr<RG::ReadOnlyImageResource> textureArray = imgMap.FindByName ("textureArray_R_32x32");
        GVK_ASSERT (textureArray != nullptr);
        std::vector<float> pixelData (32 * 32);
        std::generate_n (pixelData.begin (), 32 * 32, [] () {
            return 0.3f;
        });
        textureArray->CopyTransitionTransfer (pixelData);
    }
    {
        std::shared_ptr<RG::ReadOnlyImageResource> textureArray = imgMap.FindByName ("textureArray_RGBA_32x32");
        GVK_ASSERT (textureArray != nullptr);
        std::vector<glm::vec4> pixelData (32 * 32);
        std::generate_n (pixelData.begin (), 32 * 32, [] () {
            return glm::vec4 (0.7, 0.6, 0.8, 1.0);
        });
        textureArray->CopyTransitionTransfer (pixelData);
    }

    graph.Submit (0);

    env->Wait ();

    CompareImages ("textureArray", *presented->GetImages ()[0], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HeadlessTestEnvironment, RenderGraph_SingleOperation_SingleOutput_RenderRedImage)
{
    const std::string fragSrc = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (1, 0, 0, 1);
}
    )";

    std::shared_ptr<RG::RenderOperation> redFillOperation = RG::RenderOperation::Builder (GetDevice ())
                                                                .SetVertices (std::make_unique<RG::DrawRecordableInfo> (1, 6))
                                                                .SetPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                                                                .SetVertexShader (passThroughVertexShader)
                                                                .SetFragmentShader (fragSrc)
                                                                .Build ();

    std::shared_ptr<RG::WritableImageResource> red = std::make_unique<RG::WritableImageResource> (512, 512);

    RG::GraphSettings s (GetDeviceExtra (), 3);

    auto& aTable2 = redFillOperation->compileSettings.attachmentProvider;
    aTable2->table.push_back ({ "outColor", GVK::ShaderKind::Fragment, { red->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, red->GetImageViewForFrameProvider (), red->GetInitialLayout (), red->GetFinalLayout () } });

    s.connectionSet.Add (redFillOperation, red);

    RG::RenderGraph graph;
    graph.Compile (std::move (s));
    graph.Submit (0);
    graph.Submit (1);
    graph.Submit (2);

    env->Wait ();

    CompareImages ("red", *red->GetImages ()[0], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    CompareImages ("red", *red->GetImages ()[1], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    CompareImages ("red", *red->GetImages ()[2], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


#if 0
TEST_F (HeadlessTestEnvironment, RenderGraph_TransferOperation)
{
    const std::string fragSrc = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (1, 0, 0, 1);
}
    )";

    std::shared_ptr<RG::RenderOperation> redFillOperation = RG::RenderOperation::Builder (GetDevice ())
                                                                .SetVertices (std::make_unique<RG::DrawRecordableInfo> (1, 6))
                                                                .SetPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                                                                .SetVertexShader (passThroughVertexShader)
                                                                .SetFragmentShader (fragSrc)
                                                                .Build ();

    std::shared_ptr<RG::TransferOperation> transfer = std::make_unique<RG::TransferOperation> ();

    std::shared_ptr<RG::WritableImageResource> red       = std::make_unique<RG::WritableImageResource> (512, 512);
    std::shared_ptr<RG::WritableImageResource> duplicate = std::make_unique<RG::WritableImageResource> (512, 512);


    RG::GraphSettings s (GetDeviceExtra (), 1);

    s.connectionSet.Add (RG::OutputBuilder ()
                             .SetOperation (redFillOperation)
                             .SetTarget (red)
                             .SetBinding (0)
                             .SetClear ()
                             .Build ());

    s.connectionSet.Add (red, transfer);

    s.connectionSet.Add (RG::OutputBuilder ()
                             .SetOperation (transfer)
                             .SetTarget (duplicate)
                             .SetBinding (0)
                             .SetClear ()
                             .Build ());

    RG::RenderGraph graph;
    graph.Compile (std::move (s));
    graph.Submit (0);

    env->Wait ();

    CompareImages ("red", *duplicate->GetImages ()[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}
#endif


TEST_F (HeadlessTestEnvironment, RenderGraph_MultipleOperations_MultipleOutputs)
{
    std::shared_ptr<RG::WritableImageResource> presented = std::make_unique<RG::WritableImageResource> (512, 512);
    std::shared_ptr<RG::WritableImageResource> green     = std::make_unique<RG::WritableImageResource> (512, 512);
    std::shared_ptr<RG::WritableImageResource> red       = std::make_unique<RG::WritableImageResource> (512, 512);
    std::shared_ptr<RG::WritableImageResource> finalImg  = std::make_unique<RG::WritableImageResource> (512, 512);


    /*
        green -> dummyPass --> presented -> secondPass -> finalImg
                          \
                           \-> red
    */

    std::shared_ptr<RG::RenderOperation> dummyPass = RG::RenderOperation::Builder (GetDevice ())
                                                         .SetPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                                                         .SetVertices (std::make_unique<RG::DrawRecordableInfo> (1, 3))
                                                         .SetVertexShader (ShadersFolder / "test.vert")
                                                         .SetVertexShader (ShadersFolder / "test.frag")
                                                         .Build ();

    std::shared_ptr<RG::RenderOperation> secondPass = RG::RenderOperation::Builder (GetDevice ())
                                                          .SetPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                                                          .SetVertices (std::make_unique<RG::DrawRecordableInfo> (1, 3))
                                                          .SetVertexShader (ShadersFolder / "fullscreenquad.vert")
                                                          .SetVertexShader (ShadersFolder / "fullscreenquad.frag")
                                                          .Build ();

    auto& table = dummyPass->compileSettings.descriptorWriteProvider;
    table->imageInfos.push_back ({ "sampl", GVK::ShaderKind::Fragment, green->GetSamplerProvider (), green->GetImageViewForFrameProvider (), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });

    auto& table2 = secondPass->compileSettings.descriptorWriteProvider;
    table2->imageInfos.push_back ({ "sampl", GVK::ShaderKind::Fragment, red->GetSamplerProvider (), red->GetImageViewForFrameProvider (), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });

    auto& aTable = dummyPass->compileSettings.attachmentProvider;
    aTable->table.push_back ({ "outColor", GVK::ShaderKind::Fragment, { presented->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, presented->GetImageViewForFrameProvider (), presented->GetInitialLayout (), presented->GetFinalLayout () } });
    aTable->table.push_back ({ "red", GVK::ShaderKind::Fragment, { red->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, red->GetImageViewForFrameProvider (), red->GetInitialLayout (), red->GetFinalLayout () } });

    auto& aTable2 = secondPass->compileSettings.attachmentProvider;
    aTable2->table.push_back ({ "outColor", GVK::ShaderKind::Fragment, { finalImg->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, finalImg->GetImageViewForFrameProvider (), finalImg->GetInitialLayout (), finalImg->GetFinalLayout () } });

    RG::ConnectionSet connectionSet;
    connectionSet.Add (green, dummyPass);
    connectionSet.Add (red, secondPass);
    connectionSet.Add (dummyPass, presented);
    connectionSet.Add (dummyPass, red);
    connectionSet.Add (secondPass, finalImg);

    RG::RenderGraph graph;
    graph.Compile (RG::GraphSettings (GetDeviceExtra (), std::move (connectionSet), 4));

    for (uint32_t i = 0; i < 1; ++i) {
        graph.Submit (i);
    }

    env->Wait ();

    ASSERT_TRUE (GVK::ImageData (GetDeviceExtra (), *presented->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) == GVK::ImageData (ReferenceImagesFolder / "black.png"));
}


TEST_F (HeadlessTestEnvironment, InputAttachment_ShaderModule_Compile)
{
    std::unique_ptr<GVK::ShaderModule> shaderModule = GVK::ShaderModule::CreateFromGLSLString (GetDevice (), GVK::ShaderKind::Fragment, R"(
#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput inputDepth;

layout (binding = 2) uniform UBO {
        vec2 brightnessContrast;
        vec2 range;
        int attachmentIndex;
} ubo;

layout (location = 0) out vec4 outColor;

vec3 brightnessContrast(vec3 color, float brightness, float contrast) {
        return (color - 0.5) * contrast + 0.5 + brightness;
}

void main() 
{
    // Apply brightness and contrast filer to color input
    if (ubo.attachmentIndex == 0) {
        // Read color from previous color input attachment
        vec3 color = subpassLoad(inputColor).rgb;
        outColor.rgb = brightnessContrast(color, ubo.brightnessContrast[0], ubo.brightnessContrast[1]);
    }

    // Visualize depth input range
    if (ubo.attachmentIndex == 1) {
        // Read depth from previous depth input attachment
        float depth = subpassLoad(inputDepth).r;
        outColor.rgb = vec3((depth - ubo.range[0]) * 1.0 / (ubo.range[1] - ubo.range[0]));
    }
}
)");
}


TEST_F (HeadlessTestEnvironment, InputAttachment_RenderGraph)
{
    const std::string fragSrc = R"(
#version 450

layout (input_attachment_index = 0, binding = 0) uniform usubpassInput inputColor;

layout (location = 0) out uvec4 outColor;

void main() 
{
    uvec3 si = subpassLoad (inputColor).bgr;
    outColor = uvec4 (si, 255);
}
)";

    std::shared_ptr<RG::RenderOperation> firstPass = RG::RenderOperation::Builder (GetDevice ())
                                                         .SetPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                                                         .SetVertices (std::make_unique<RG::DrawRecordableInfo> (1, 6))
                                                         .SetVertexShader (passThroughVertexShader)
                                                         .SetFragmentShader (fragSrc)
                                                         .SetBlendEnabled (false)
                                                         .Build ();

    std::shared_ptr<RG::WritableImageResource> inputColor = std::make_unique<RG::WritableImageResource> (VK_FILTER_LINEAR, 32, 32, 1, VK_FORMAT_R8G8B8A8_UINT);

    std::shared_ptr<RG::WritableImageResource> outColor = std::make_unique<RG::WritableImageResource> (VK_FILTER_LINEAR, 32, 32, 1, VK_FORMAT_R8G8B8A8_UINT);

    inputColor->initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    inputColor->finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    auto& aTable = firstPass->compileSettings.attachmentProvider->table;
    aTable.push_back ({ "inputColor", GVK::ShaderKind::Fragment, { inputColor->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_LOAD, inputColor->GetImageViewForFrameProvider (), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL } });
    aTable.push_back ({ "outColor", GVK::ShaderKind::Fragment, { outColor->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, outColor->GetImageViewForFrameProvider (), outColor->GetInitialLayout (), outColor->GetFinalLayout () } });

    auto& dTable = firstPass->compileSettings.descriptorWriteProvider;
    dTable->imageInfos.push_back ({ "inputColor", GVK::ShaderKind::Fragment, inputColor->GetSamplerProvider (), inputColor->GetImageViewForFrameProvider (), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });

    RG::ConnectionSet connectionSet;

    connectionSet.Add (inputColor, firstPass);
    connectionSet.Add (firstPass, outColor);

    RG::GraphSettings s;

    s.framesInFlight = 1;
    s.device         = &GetDeviceExtra ();
    s.connectionSet  = std::move (connectionSet);

    RG::RenderGraph graph;
    graph.Compile (std::move (s));

    {
        GVK::Buffer buff (*env->allocator, 32 * 32 * 4 * 3, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, GVK::Buffer::MemoryLocation::CPU);

        GVK::MemoryMapping mapping (*env->allocator, buff);

        std::vector<uint8_t> pixelData (32 * 32 * 4 * 3);
        for (uint32_t i = 0; i < pixelData.size (); i += 4) {
            pixelData[i + 0] = 255;
            pixelData[i + 1] = 0;
            pixelData[i + 2] = 0;
            pixelData[i + 3] = 128;
        }
        mapping.Copy (pixelData);

        {
            GVK::SingleTimeCommand s (GetDevice (), GetCommandPool (), GetGraphicsQueue ());
            s.Record<GVK::CommandTranstionImage> (*inputColor->GetImages ()[0], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            s.Record<GVK::CommandCopyBufferToImage> (buff, *inputColor->GetImages ()[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, std::vector<VkBufferImageCopy> { inputColor->GetImages ()[0]->GetFullBufferImageCopy () });
            s.Record<GVK::CommandTranstionImage> (*inputColor->GetImages ()[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }


    env->Wait ();
    graph.Submit (0);
    env->Wait ();

    GVK::ImageData img (GetDeviceExtra (), *outColor->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    GVK::ImageData referenceImage (ReferenceImagesFolder / "blue_32x32.png");

    EXPECT_TRUE (referenceImage == img);
}


TEST_F (HeadlessTestEnvironment, DISABLED_RenderGraph_SameImageAsInputAndOutput)
{
    /*
        presented ---> firstPass  ---> presented
    */

    const std::string frag1 = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) flat in uvec4 inColor[3];

layout (location = 0) out uvec4 outColor[3];

void main () {
    outColor[0] = uvec4 (inColor[0].bgr, 255);
}
    )";

    std::shared_ptr<RG::RenderOperation> firstPass = RG::RenderOperation::Builder (GetDevice ())
                                                         .SetPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                                                         .SetVertices (std::make_unique<RG::DrawRecordableInfo> (1, 6))
                                                         .SetVertexShader (passThroughVertexShader)
                                                         .SetFragmentShader (frag1)
                                                         .SetName ("FIRST")
                                                         .Build ();

    std::shared_ptr<RG::WritableImageResource> presented = std::make_unique<RG::WritableImageResource> (VK_FILTER_NEAREST, 32, 32, 3, VK_FORMAT_R8G8B8A8_UINT);

    RG::GraphSettings s (GetDeviceExtra (), 1);

    auto& aTable2 = firstPass->compileSettings.attachmentProvider;
    aTable2->table.push_back ({ "outColor", GVK::ShaderKind::Fragment, { presented->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_LOAD, presented->GetImageViewForFrameProvider (), presented->GetInitialLayout (), presented->GetFinalLayout () } });

    auto& table = firstPass->compileSettings.descriptorWriteProvider;
    table->imageInfos.push_back ({ "inColor", GVK::ShaderKind::Fragment, presented->GetSamplerProvider (), presented->GetImageViewForFrameProvider (), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });

    s.connectionSet.Add (presented, firstPass);
    s.connectionSet.Add (firstPass, presented);

    RG::RenderGraph graph;
    graph.Compile (std::move (s));

    GVK::ImageData before2_resultImage (GetDeviceExtra (), *presented->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    before2_resultImage.SaveTo (TempFolder / "before_tempDoubl2e.png");

    {
        GVK::Buffer buff (*env->allocator, 32 * 32 * 4 * 3, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, GVK::Buffer::MemoryLocation::CPU);

        GVK::MemoryMapping mapping (*env->allocator, buff);

        std::vector<uint8_t> pixelData (32 * 32 * 4 * 3);
        for (uint32_t i = 0; i < pixelData.size (); i += 4) {
            pixelData[i + 0] = 255;
            pixelData[i + 1] = 0;
            pixelData[i + 2] = 0;
            pixelData[i + 3] = 128;
        }
        mapping.Copy (pixelData);

        {
            GVK::SingleTimeCommand s (GetDevice (), GetCommandPool (), GetGraphicsQueue ());
            s.Record<GVK::CommandTranstionImage> (*presented->GetImages ()[0], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            s.Record<GVK::CommandCopyBufferToImage> (buff, *presented->GetImages ()[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, std::vector<VkBufferImageCopy> { presented->GetImages ()[0]->GetFullBufferImageCopy () });
            s.Record<GVK::CommandTranstionImage> (*presented->GetImages ()[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }
    }

    env->Wait ();
    GVK::ImageData before_resultImage (GetDeviceExtra (), *presented->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    before_resultImage.SaveTo (TempFolder / "before_tempDouble.png");

    graph.Submit (0);
    env->Wait ();

    GVK::ImageData resultImage (GetDeviceExtra (), *presented->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    resultImage.SaveTo (TempFolder / "tempDouble.png");
}


TEST_F (HeadlessTestEnvironment, RenderGraph_TwoOperationsRenderingToOutput)
{
    /*
        firstPass  ---> presented
        secondPass  _/ 
    */

    // will turn into

    /*
        firstPass ---> presented ---> secondPass ---> presented
    */

    const std::string frag1 = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (1, 0, 0, 0.5);
}
    )";

    const std::string frag2 = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (0, 0, 1, 0.5);
}
    )";

    std::shared_ptr<RG::RenderOperation> firstPass = RG::RenderOperation::Builder (GetDevice ())
                                                         .SetPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                                                         .SetVertices (std::make_unique<RG::DrawRecordableInfo> (1, 6))
                                                         .SetVertexShader (passThroughVertexShader)
                                                         .SetFragmentShader (frag1)
                                                         .SetBlendEnabled (false)
                                                         .SetName ("FIRST")
                                                         .Build ();

    std::shared_ptr<RG::RenderOperation> secondPass = RG::RenderOperation::Builder (GetDevice ())
                                                          .SetPrimitiveTopology (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                                                          .SetVertices (std::make_unique<RG::DrawRecordableInfo> (1, 6))
                                                          .SetVertexShader (passThroughVertexShader)
                                                          .SetFragmentShader (frag2)
                                                          .SetBlendEnabled (true)
                                                          .SetName ("SECOND")
                                                          .Build ();

    std::shared_ptr<RG::WritableImageResource> presented = std::make_unique<RG::WritableImageResource> (VK_FILTER_LINEAR, 512, 512, 1, VK_FORMAT_R8G8B8A8_SRGB);

    RG::GraphSettings s (GetDeviceExtra (), 1);

    auto& aTable2 = firstPass->compileSettings.attachmentProvider;
    aTable2->table.push_back ({ "outColor", GVK::ShaderKind::Fragment, { presented->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, presented->GetImageViewForFrameProvider (), presented->GetInitialLayout (), presented->GetFinalLayout () } });

    s.connectionSet.Add (firstPass, presented);

    auto& aTable3 = secondPass->compileSettings.attachmentProvider;
    aTable3->table.push_back ({ "outColor", GVK::ShaderKind::Fragment, { presented->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_LOAD, presented->GetImageViewForFrameProvider (), presented->GetInitialLayout (), presented->GetFinalLayout () } });

    s.connectionSet.Add (secondPass, presented);

    RG::RenderGraph graph;
    graph.Compile (std::move (s));

    EXPECT_EQ (2, graph.GetPassCount ());

    GVK::ImageData referenceImage (ReferenceImagesFolder / "pink.png");

    const size_t renderCount = 1;
    size_t       matchCount  = 0;

    for (size_t i = 0; i < renderCount; ++i) {
        std::this_thread::sleep_for (std::chrono::milliseconds (200));

        graph.Submit (0);

        env->Wait ();

        GVK::ImageData (GetDeviceExtra (), *presented->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).SaveTo (ReferenceImagesFolder / "presentedTwo.png");

        if (GVK::ImageData (GetDeviceExtra (), *presented->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) == referenceImage) {
            ++matchCount;
        }
    }

    EXPECT_EQ (renderCount, matchCount);
}


// no window, swapchain, surface
class HeadlessTestEnvironmentWithExt : public TestEnvironmentBase {
protected:
    virtual void SetUp () override;
    virtual void TearDown () override;
};


void HeadlessTestEnvironmentWithExt::SetUp ()
{
    env = std::make_unique<RG::VulkanEnvironment> (testDebugCallback, RG::GetGLFWInstanceExtensions (), std::vector<const char*> { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
}


void HeadlessTestEnvironmentWithExt::TearDown ()
{
    env.reset ();
}


TEST_F (HeadlessTestEnvironmentWithExt, Swapchain_Create)
{
    RG::GLFWWindow window;

    GVK::Surface surface (*env->instance, window.GetSurface (*env->instance));

    GVK_ASSERT (env->physicalDevice->CheckSurfaceSupported (surface));

    GVK::RealSwapchain swapchain (*env->physicalDevice, *env->device, surface, std::make_unique<GVK::DefaultSwapchainSettings> ());
}


TEST_F (HiddenWindowTestEnvironment, RenderGraph_RenderingToSwapchain)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();
    GVK::Swapchain&   swapchain     = GetSwapchain ();


    RG::GraphSettings s (device, swapchain.GetImageCount ());
    RG::RenderGraph   graph;

    auto sp = std::make_unique<RG::ShaderPipeline> (device);
    sp->SetVertexShaderFromString (passThroughVertexShader);

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

    std::shared_ptr<RG::RenderOperation> redFillOperation = std::make_unique<RG::RenderOperation> (std::make_unique<RG::DrawRecordableInfo> (1, 6), std::move (sp));

    std::shared_ptr<RG::WritableImageResource> presentedCopy = std::make_unique<RG::WritableImageResource> (800, 600);
    std::shared_ptr<RG::SwapchainImageResource> presented     = std::make_unique<RG::SwapchainImageResource> (*presentable);

    auto& aTable3 = redFillOperation->compileSettings.attachmentProvider;
    aTable3->table.push_back ({ "outColor", GVK::ShaderKind::Fragment, { presented->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, presented->GetImageViewForFrameProvider (), presented->GetInitialLayout (), presented->GetFinalLayout () } });
    aTable3->table.push_back ({ "outCopy", GVK::ShaderKind::Fragment, { presentedCopy->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, presentedCopy->GetImageViewForFrameProvider (), presentedCopy->GetInitialLayout (), presentedCopy->GetFinalLayout () } });

    s.connectionSet.Add (redFillOperation, presented);
    s.connectionSet.Add (redFillOperation, presentedCopy);

    graph.Compile (std::move (s));

    RG::BlockingGraphRenderer renderer (device, swapchain);
    uint32_t                  count = 0;
    auto                      cb    = renderer.GetConditionalDrawCallback ([&] () -> RG::RenderGraph& { return graph; }, [&] () -> bool { return ++count > 10; });
    window->DoEventLoop (cb);
}


TEST_F (HiddenWindowTestEnvironment, RenderGraph_VertexAndIndexBuffer)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();
    GVK::Swapchain&   swapchain     = GetSwapchain ();

    RG::GraphSettings s (device, swapchain.GetImageCount ());
    RG::RenderGraph   graph;

    auto sp = std::make_unique<RG::ShaderPipeline> (device);
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

    std::shared_ptr<RG::RenderOperation> redFillOperation = std::make_unique<RG::RenderOperation> (std::make_unique<RG::DrawRecordableInfo> (1, static_cast<uint32_t> (vbb.data.size ()), vbb.buffer.GetBufferToBind (), vbb.info.bindings, vbb.info.attributes, static_cast<uint32_t> (ib.data.size ()), ib.buffer.GetBufferToBind ()),
                                                                                                   std::move (sp));

    std::shared_ptr<RG::WritableImageResource>  presentedCopy = std::make_unique<RG::WritableImageResource> (800, 600);
    std::shared_ptr<RG::SwapchainImageResource> presented     = std::make_unique<RG::SwapchainImageResource> (*presentable);

    auto& aTable2 = redFillOperation->compileSettings.attachmentProvider;
    aTable2->table.push_back ({ "outColor", GVK::ShaderKind::Fragment, { presented->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, presented->GetImageViewForFrameProvider (), presented->GetInitialLayout (), presented->GetFinalLayout () } });
    aTable2->table.push_back ({ "outCopy", GVK::ShaderKind::Fragment, { presentedCopy->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, presentedCopy->GetImageViewForFrameProvider (), presentedCopy->GetInitialLayout (), presentedCopy->GetFinalLayout () } });

    s.connectionSet.Add (redFillOperation, presented);
    s.connectionSet.Add (redFillOperation, presentedCopy);

    graph.Compile (std::move (s));

    RG::BlockingGraphRenderer renderer (device, swapchain);
    uint32_t                  count = 0;
    auto                      cb    = renderer.GetConditionalDrawCallback ([&] () -> RG::RenderGraph& { return graph; }, [&] () -> bool { return ++count > 10; });
    window->DoEventLoop (cb);

    CompareImages ("uv", *presentedCopy->images[0]->image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HiddenWindowTestEnvironment, RenderGraph_BasicUniformBuffer)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();
    GVK::Swapchain&   swapchain     = GetSwapchain ();

    RG::GraphSettings s (device, swapchain.GetImageCount ());
    RG::RenderGraph   graph;

    auto sp = std::make_unique<RG::ShaderPipeline> (device);
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

    std::shared_ptr<RG::RenderOperation> redFillOperation = std::make_unique<RG::RenderOperation> (std::make_unique<RG::DrawRecordableInfo> (1, *vbb, ib), std::move (sp));

    std::shared_ptr<RG::SwapchainImageResource> presented     = std::make_unique<RG::SwapchainImageResource> (*presentable);
    std::shared_ptr<RG::WritableImageResource>  presentedCopy = std::make_unique<RG::WritableImageResource> (VK_FILTER_LINEAR, 800, 600, 2);
    std::shared_ptr<RG::CPUBufferResource>      unif          = std::make_unique<RG::CPUBufferResource> (4);

    s.connectionSet.Add (unif);

    auto& aTable2 = redFillOperation->compileSettings.attachmentProvider;
    aTable2->table.push_back ({ "presented", GVK::ShaderKind::Fragment, { presented->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, presented->GetImageViewForFrameProvider (), presented->GetInitialLayout (), presented->GetFinalLayout () } });
    aTable2->table.push_back ({ "copy", GVK::ShaderKind::Fragment, { presentedCopy->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, presentedCopy->GetImageViewForFrameProvider (), presentedCopy->GetInitialLayout (), presentedCopy->GetFinalLayout () } });

    s.connectionSet.Add (redFillOperation, presented);
    s.connectionSet.Add (redFillOperation, presentedCopy);

    auto& table = redFillOperation->compileSettings.descriptorWriteProvider;
    table->bufferInfos.push_back ({ std::string ("Time"), GVK::ShaderKind::Vertex, unif->GetBufferForFrameProvider (), 0, unif->GetBufferSize () });

    graph.Compile (std::move (s));

    RG::BlockingGraphRenderer renderer (device, swapchain);

    GVK::EventObserver obs;
    obs.Observe (renderer.preSubmitEvent, [&] (RG::RenderGraph&, uint32_t frameIndex, uint64_t) {
        float time = 0.5f;
        unif->GetMapping (frameIndex).Copy (time);
    });

    uint32_t count = 0;
    auto     cb    = renderer.GetConditionalDrawCallback ([&] () -> RG::RenderGraph& { return graph; }, [&] () -> bool { return ++count > 10; });
    window->DoEventLoop (cb);

    CompareImages ("uvoffset", *presentedCopy->images[0]->image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}
