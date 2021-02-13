#include <vulkan/vulkan.h>

#include "FullscreenQuad.hpp"
#include "GraphRenderer.hpp"
#include "GraphSettings.hpp"
#include "Operation.hpp"
#include "Ptr.hpp"
#include "RenderGraph.hpp"
#include "Resource.hpp"
#include "ShaderPipeline.hpp"
#include "Timer.hpp"
#include "UniformReflection.hpp"
#include "Utils.hpp"
#include "VulkanEnvironment.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "glmlib.hpp"

#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include "GoogleTestEnvironment.hpp"
#include "ImageData.hpp"


const std::filesystem::path ShadersFolder = PROJECT_ROOT / "TestData" / "shaders";


#include "SourceLocation.hpp"
#include "UniformView.hpp"


TEST_F (HeadlessGoogleTestEnvironment, Spirvrross2)
{
    U<GVK::ShaderModule> sm = GVK::ShaderModule::CreateFromGLSLString (GetDevice (), GVK::ShaderKind::Fragment, R"(#version 450

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

    GVK::RG::RenderOperation op (Make<GVK::DrawRecordableInfo> (1, 3),
                                 Make<GVK::RG::ShaderPipeline> (device, std::vector<std::filesystem::path> {
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

    } catch (GVK::ShaderCompileException& e) {
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

    auto sp = Make<GVK::RG::ShaderPipeline> (device);
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

    Ptr<GVK::RG::RenderOperation> redFillOperation = Make<GVK::RG::RenderOperation> (Make<GVK::DrawRecordableInfo> (1, 6), std::move (sp));

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

    Ptr<GVK::RG::WritableImageResource> presented = Make<GVK::RG::WritableImageResource> (512, 512);

    s.connectionSet.Add (redFillOperation, presented,
                         Make<GVK::RG::OutputBinding> (0,
                                                       presented->GetFormatProvider (),
                                                       presented->GetFinalLayout (),
                                                       VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                       VK_ATTACHMENT_STORE_OP_STORE));

    graph.Compile (std::move (s));

    {
        Ptr<GVK::RG::ReadOnlyImageResource> textureArray = imgMap.FindByName ("textureArray_R_32x32");
        GVK_ASSERT (textureArray != nullptr);
        std::vector<float> pixelData (32 * 32);
        std::generate_n (pixelData.begin (), 32 * 32, [] () {
            return 0.3;
        });
        textureArray->CopyTransitionTransfer (pixelData);
    }
    {
        Ptr<GVK::RG::ReadOnlyImageResource> textureArray = imgMap.FindByName ("textureArray_RGBA_32x32");
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

    auto sp = Make<GVK::RG::ShaderPipeline> (device);
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

    Ptr<GVK::RG::RenderOperation> redFillOperation = Make<GVK::RG::RenderOperation> (Make<GVK::DrawRecordableInfo> (1, 6), std::move (sp));

    Ptr<GVK::RG::ImageResource> red = Make<GVK::RG::WritableImageResource> (512, 512);

    s.connectionSet.Add (redFillOperation, red,
                         Make<GVK::RG::OutputBinding> (0,
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

    auto sp = Make<GVK::RG::ShaderPipeline> (device);
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

    Ptr<GVK::RG::RenderOperation> redFillOperation = Make<GVK::RG::RenderOperation> (Make<GVK::DrawRecordableInfo> (1, 6), std::move (sp));

    Ptr<GVK::RG::TransferOperation> transfer = Make<GVK::RG::TransferOperation> ();

    Ptr<GVK::RG::WritableImageResource> red       = Make<GVK::RG::WritableImageResource> (512, 512);
    Ptr<GVK::RG::WritableImageResource> duplicate = Make<GVK::RG::WritableImageResource> (512, 512);


    s.connectionSet.Add (redFillOperation, red,
                         Make<GVK::RG::OutputBinding> (0,
                                                       red->GetFormatProvider (),
                                                       red->GetFinalLayout (),
                                                       VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                       VK_ATTACHMENT_STORE_OP_STORE));


    s.connectionSet.Add (red, transfer,
                         Make<GVK::RG::ImageInputBinding> (0, *red));

    s.connectionSet.Add (transfer, duplicate,
                         Make<GVK::RG::OutputBinding> (0,
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

    Ptr<GVK::RG::WritableImageResource> presented = Make<GVK::RG::WritableImageResource> (512, 512);
    Ptr<GVK::RG::WritableImageResource> green     = Make<GVK::RG::WritableImageResource> (512, 512);
    Ptr<GVK::RG::WritableImageResource> red       = Make<GVK::RG::WritableImageResource> (512, 512);
    Ptr<GVK::RG::WritableImageResource> finalImg  = Make<GVK::RG::WritableImageResource> (512, 512);


    Ptr<GVK::RG::RenderOperation> dummyPass = Make<GVK::RG::RenderOperation> (Make<GVK::DrawRecordableInfo> (1, 3),
                                                                              Make<GVK::RG::ShaderPipeline> (device, std::vector<std::filesystem::path> {
                                                                                                                         ShadersFolder / "test.vert",
                                                                                                                         ShadersFolder / "test.frag",
                                                                                                                     }));

    Ptr<GVK::RG::RenderOperation> secondPass = Make<GVK::RG::RenderOperation> (Make<GVK::DrawRecordableInfo> (1, 3),
                                                                               Make<GVK::RG::ShaderPipeline> (device, std::vector<std::filesystem::path> {
                                                                                                                          ShadersFolder / "fullscreenquad.vert",
                                                                                                                          ShadersFolder / "fullscreenquad.frag",
                                                                                                                      }));


    s.connectionSet.Add (green, dummyPass, Make<GVK::RG::ImageInputBinding> (0, *green));
    s.connectionSet.Add (red, secondPass, Make<GVK::RG::ImageInputBinding> (0, *red));

    s.connectionSet.Add (dummyPass, presented,
                         Make<GVK::RG::OutputBinding> (0,
                                                       presented->GetFormatProvider (),
                                                       presented->GetFinalLayout (),
                                                       VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                       VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (dummyPass, red,
                         Make<GVK::RG::OutputBinding> (1,
                                                       red->GetFormatProvider (),
                                                       red->GetFinalLayout (),
                                                       VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                       VK_ATTACHMENT_STORE_OP_STORE));


    s.connectionSet.Add (secondPass, finalImg,
                         Make<GVK::RG::OutputBinding> (0,
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


TEST_F (HiddenWindowGoogleTestEnvironment, SwapchainTest)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();
    GVK::Swapchain&   swapchain     = GetSwapchain ();


    GVK::RG::GraphSettings s (device, swapchain.GetImageCount ());
    GVK::RG::RenderGraph   graph;

    auto sp = Make<GVK::RG::ShaderPipeline> (device);
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

    Ptr<GVK::RG::RenderOperation> redFillOperation = Make<GVK::RG::RenderOperation> (Make<GVK::DrawRecordableInfo> (1, 6), std::move (sp));

    Ptr<GVK::RG::ImageResource> presentedCopy = Make<GVK::RG::WritableImageResource> (800, 600);
    Ptr<GVK::RG::ImageResource> presented     = Make<GVK::RG::SwapchainImageResource> (*presentable);

    s.connectionSet.Add (redFillOperation, presented,
                         Make<GVK::RG::OutputBinding> (0,
                                                       presented->GetFormatProvider (),
                                                       presented->GetFinalLayout (),
                                                       VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                       VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (redFillOperation, presentedCopy,
                         Make<GVK::RG::OutputBinding> (1,
                                                       presentedCopy->GetFormatProvider (),
                                                       presentedCopy->GetFinalLayout (),
                                                       VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                       VK_ATTACHMENT_STORE_OP_STORE));
    graph.Compile (std::move (s));

    GVK::RG::BlockingGraphRenderer renderer (device, swapchain);
    window->DoEventLoop (renderer.GetCountLimitedDrawCallback ([&] () -> GVK::RG::RenderGraph& { return graph; }, 10));
}


TEST_F (HiddenWindowGoogleTestEnvironment, VertexAndIndexBufferTest)
{
    GVK::DeviceExtra& device        = GetDeviceExtra ();
    GVK::CommandPool& commandPool   = GetCommandPool ();
    GVK::Queue&       graphicsQueue = GetGraphicsQueue ();
    GVK::Swapchain&   swapchain     = GetSwapchain ();

    GVK::RG::GraphSettings s (device, swapchain.GetImageCount ());
    GVK::RG::RenderGraph   graph;

    auto sp = Make<GVK::RG::ShaderPipeline> (device);
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

    Ptr<GVK::RG::RenderOperation> redFillOperation = Make<GVK::RG::RenderOperation> (Make<GVK::DrawRecordableInfo> (1, vbb.data.size (), vbb.buffer.GetBufferToBind (), vbb.info.bindings, vbb.info.attributes, ib.data.size (), ib.buffer.GetBufferToBind ()),
                                                                                     std::move (sp));

    Ptr<GVK::RG::WritableImageResource>  presentedCopy = Make<GVK::RG::WritableImageResource> (800, 600);
    Ptr<GVK::RG::SwapchainImageResource> presented     = Make<GVK::RG::SwapchainImageResource> (*presentable);


    s.connectionSet.Add (redFillOperation, presented,
                         Make<GVK::RG::OutputBinding> (0,
                                                       presented->GetFormatProvider (),
                                                       presented->GetFinalLayout (),
                                                       VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                       VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (redFillOperation, presentedCopy,
                         Make<GVK::RG::OutputBinding> (1,
                                                       presentedCopy->GetFormatProvider (),
                                                       presentedCopy->GetFinalLayout (),
                                                       VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                       VK_ATTACHMENT_STORE_OP_STORE));

    graph.Compile (std::move (s));

    GVK::RG::BlockingGraphRenderer renderer (device, swapchain);
    window->DoEventLoop (renderer.GetCountLimitedDrawCallback ([&] () -> GVK::RG::RenderGraph& { return graph; }, 10));

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

    auto sp = Make<GVK::RG::ShaderPipeline> (device);
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

    Ptr<GVK::VertexBufferTransferable<Vert>> vbb = Make<GVK::VertexBufferTransferable<Vert>> (
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

    Ptr<GVK::RG::RenderOperation> redFillOperation = Make<GVK::RG::RenderOperation> (Make<GVK::DrawRecordableInfo> (1, *vbb, ib), std::move (sp));

    Ptr<GVK::RG::SwapchainImageResource> presented     = Make<GVK::RG::SwapchainImageResource> (*presentable);
    Ptr<GVK::RG::WritableImageResource>  presentedCopy = Make<GVK::RG::WritableImageResource> (VK_FILTER_LINEAR, 800, 600, 2);
    Ptr<GVK::RG::CPUBufferResource>      unif          = Make<GVK::RG::CPUBufferResource> (4);


    s.connectionSet.Add (unif, redFillOperation,
                         Make<GVK::RG::UniformInputBinding> (0, *unif));

    s.connectionSet.Add (redFillOperation, presented,
                         Make<GVK::RG::OutputBinding> (2,
                                                       presented->GetFormatProvider (),
                                                       presented->GetFinalLayout (),
                                                       VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                       VK_ATTACHMENT_STORE_OP_STORE));


    s.connectionSet.Add (redFillOperation, presentedCopy,
                         Make<GVK::RG::OutputBinding> (0,
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

    window->DoEventLoop (renderer.GetCountLimitedDrawCallback ([&] () -> GVK::RG::RenderGraph& { return graph; }, 10));

    CompareImages ("uvoffset", *presentedCopy->images[0]->image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}
