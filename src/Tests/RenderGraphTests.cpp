#include <vulkan/vulkan.h>

#include "FullscreenQuad.hpp"
#include "GraphRenderer.hpp"
#include "Ptr.hpp"
#include "GraphSettings.hpp"
#include "RenderGraph.hpp"
#include "ShaderPipeline.hpp"
#include "Timer.hpp"
#include "UniformReflection.hpp"
#include "Utils.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"
#include "Operation.hpp"
#include "Resource.hpp"

#include "glmlib.hpp"

#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include "GoogleTestEnvironment.hpp"
#include "ImageData.hpp"


const std::filesystem::path ShadersFolder = PROJECT_ROOT / "src" / "Tests" / "shaders";


using namespace RG;

#include "UniformView.hpp"


TEST_F (HiddenWindowGoogleTestEnvironment, Spirvrross2)
{
    auto            sm = ShaderModule::CreateFromGLSLString (GetDevice (), ShaderKind::Fragment, R"(#version 450

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
    SR::ShaderUData refl (sm);

    refl["Quadrics"]["quadrics"][0]["WTF"] = glm::mat3x4 ();


    EXPECT_EQ (4256, refl.GetUbo ("Quadrics")->GetFullSize ());
}


TEST_F (HeadlessGoogleTestEnvironment, CompileTest)
{
    DeviceExtra& device = GetDeviceExtra ();

    RenderOperation op (DrawRecordableInfo::Create (1, 3),
                        ShaderPipeline::Create (device, std::vector<std::filesystem::path> {
                                                            ShadersFolder / "test.vert",
                                                            ShadersFolder / "test.frag",
                                                        }));
}


TEST_F (HeadlessGoogleTestEnvironment, ShaderCompileTests)
{
    try {
        ShaderModule::CreateFromGLSLString (GetDevice (), ShaderKind::Vertex, R"(
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
    } catch (ShaderCompileException& e) {
        std::cout << e.what () << std::endl;
    }
}


TEST_F (HeadlessGoogleTestEnvironment, ImageMap_TextureArray)
{
    DeviceExtra& device        = GetDeviceExtra ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    GraphSettings s (device, 3);
    RenderGraph   graph;

    auto sp = ShaderPipeline::CreateShared (device);
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

    RenderOperationP redFillOperation = RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 6), sp);

    s.connectionSet.Add (redFillOperation);

    ImageMap imgMap = CreateEmptyImageResources (s.connectionSet, [&] (const SR::Sampler& sampler) -> std::optional<CreateParams> {
        if (sampler.name == "textureArray_RGBA_32x32") {
            return std::make_tuple (glm::uvec3 { 32, 32, 0 }, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FILTER_NEAREST);
        }
        if (sampler.name == "textureArray_R_32x32") {
            return std::make_tuple (glm::uvec3 { 32, 32, 0 }, VK_FORMAT_R32_SFLOAT, VK_FILTER_NEAREST);
        }
        return std::nullopt;
    });

    WritableImageResourceP presented = WritableImageResource::Create (512, 512);

    s.connectionSet.Add (redFillOperation, presented,
                         RG::OutputBinding::Create (0,
                                                    presented->GetFormatProvider (),
                                                    presented->GetFinalLayout (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));

    graph.Compile (s);

    {
        RG::ReadOnlyImageResourceP textureArray = imgMap.FindByName ("textureArray_R_32x32");
        GVK_ASSERT (textureArray != nullptr);
        std::vector<float> pixelData (32 * 32);
        std::generate_n (pixelData.begin (), 32 * 32, [] () {
            return 0.3;
        });
        textureArray->CopyTransitionTransfer (pixelData);
    }
    {
        RG::ReadOnlyImageResourceP textureArray = imgMap.FindByName ("textureArray_RGBA_32x32");
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
    DeviceExtra& device        = GetDeviceExtra ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    GraphSettings s (device, 3);
    RenderGraph   graph;

    auto sp = ShaderPipeline::CreateShared (device);
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

    RenderOperationP redFillOperation = RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 6), sp);

    ImageResourceP red = WritableImageResource::Create (512, 512);

    s.connectionSet.Add (redFillOperation, red,
                         RG::OutputBinding::Create (0,
                                                    red->GetFormatProvider (),
                                                    red->GetFinalLayout (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));

    graph.Compile (s);

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
    DeviceExtra& device        = GetDeviceExtra ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    GraphSettings s (device, 1);
    RenderGraph   graph;

    auto sp = ShaderPipeline::CreateShared (device);
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

    RenderOperationP redFillOperation = RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 6), sp);

    TransferOperationP transfer = TransferOperation::Create ();

    WritableImageResourceP red       = WritableImageResource::Create (512, 512);
    WritableImageResourceP duplicate = WritableImageResource::Create (512, 512);


    s.connectionSet.Add (redFillOperation, red,
                         RG::OutputBinding::Create (0,
                                                    red->GetFormatProvider (),
                                                    red->GetFinalLayout (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));


    s.connectionSet.Add (red, transfer,
                         RG::ImageInputBinding::Create (0, *red));

    s.connectionSet.Add (transfer, duplicate,
                         RG::OutputBinding::Create (0,
                                                    duplicate->GetFormatProvider (),
                                                    duplicate->GetFinalLayout (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));


    graph.Compile (s);

    graph.Submit (0);

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    CompareImages ("red", *duplicate->GetImages ()[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}


TEST_F (HeadlessGoogleTestEnvironment, RenderGraphUseTest)
{
    DeviceExtra& device        = GetDeviceExtra ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    GraphSettings s (device, 4);
    RenderGraph   graph;

    WritableImageResourceP presented = WritableImageResource::Create (512, 512);
    WritableImageResourceP green     = WritableImageResource::Create (512, 512);
    WritableImageResourceP red       = WritableImageResource::Create (512, 512);
    WritableImageResourceP finalImg  = WritableImageResource::Create (512, 512);


    RenderOperationP dummyPass = RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3),
                                                          ShaderPipeline::CreateShared (device, std::vector<std::filesystem::path> {
                                                                                                    ShadersFolder / "test.vert",
                                                                                                    ShadersFolder / "test.frag",
                                                                                                }));

    RenderOperationP secondPass = RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3),
                                                           ShaderPipeline::CreateShared (device, std::vector<std::filesystem::path> {
                                                                                                     ShadersFolder / "fullscreenquad.vert",
                                                                                                     ShadersFolder / "fullscreenquad.frag",
                                                                                                 }));


    s.connectionSet.Add (green, dummyPass, ImageInputBinding::Create (0, *green));
    s.connectionSet.Add (red, secondPass, ImageInputBinding::Create (0, *red));

    s.connectionSet.Add (dummyPass, presented,
                         RG::OutputBinding::Create (0,
                                                    presented->GetFormatProvider (),
                                                    presented->GetFinalLayout (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (dummyPass, red,
                         RG::OutputBinding::Create (1,
                                                    red->GetFormatProvider (),
                                                    red->GetFinalLayout (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));


    s.connectionSet.Add (secondPass, finalImg,
                         RG::OutputBinding::Create (0,
                                                    finalImg->GetFormatProvider (),
                                                    finalImg->GetFinalLayout (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));

    graph.Compile (s);

    Utils::DebugTimerLogger obs;
    {
        Utils::TimerScope _ (obs);
        for (uint32_t i = 0; i < 1; ++i) {
            graph.Submit (i);
        }
    }

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    ImageData (device, *green->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).SaveTo (ReferenceImagesFolder / "green.png");
    ImageData (device, *presented->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).SaveTo (ReferenceImagesFolder / "presented.png");
    ImageData (device, *red->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).SaveTo (ReferenceImagesFolder / "red.png");
    ImageData (device, *finalImg->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).SaveTo (ReferenceImagesFolder / "final.png");

    ASSERT_TRUE (ImageData (device, *presented->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) == ImageData (ReferenceImagesFolder / "black.png"));
}


TEST_F (HiddenWindowGoogleTestEnvironment, SwapchainTest)
{
    DeviceExtra& device        = GetDeviceExtra ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();


    GraphSettings s (device, swapchain.GetImageCount ());
    RenderGraph   graph;

    auto sp = ShaderPipeline::CreateShared (device);
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

    RenderOperationP redFillOperation = RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 6), sp);

    ImageResourceP presentedCopy = WritableImageResource::Create (800, 600);
    ImageResourceP presented     = SwapchainImageResource::Create (*presentable);

    s.connectionSet.Add (redFillOperation, presented,
                         RG::OutputBinding::Create (0,
                                                    presented->GetFormatProvider (),
                                                    presented->GetFinalLayout (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (redFillOperation, presentedCopy,
                         RG::OutputBinding::Create (1,
                                                    presentedCopy->GetFormatProvider (),
                                                    presentedCopy->GetFinalLayout (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));
    graph.Compile (s);

    BlockingGraphRenderer renderer (s, swapchain);
    window->DoEventLoop (renderer.GetCountLimitedDrawCallback ([&] () -> RG::RenderGraph& { return graph; }, 10));
}


TEST_F (HiddenWindowGoogleTestEnvironment, VertexAndIndexBufferTest)
{
    DeviceExtra& device        = GetDeviceExtra ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();

    GraphSettings s (device, swapchain.GetImageCount ());

    RenderGraph graph;

    auto sp = ShaderPipeline::CreateShared (device);
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

    VertexBufferTransferable<Vert> vbb (device, 4, { ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Float }, VK_VERTEX_INPUT_RATE_VERTEX);
    vbb = std::vector<Vert> {
        { glm::vec2 (-1.f, -1.f), glm::vec2 (0.f, 0.f), 0.1f },
        { glm::vec2 (-1.f, +1.f), glm::vec2 (0.f, 1.f), 0.2f },
        { glm::vec2 (+1.f, +1.f), glm::vec2 (1.f, 1.f), 0.3f },
        { glm::vec2 (+1.f, -1.f), glm::vec2 (1.f, 0.f), 0.6f },
    };
    vbb.Flush ();

    IndexBufferTransferable ib (device, 6);
    ib.data = { 0, 1, 2, 0, 3, 2 };
    ib.Flush ();

    RenderOperationP redFillOperation = RenderOperation::Create (DrawRecordableInfo::CreateShared (1, vbb.data.size (), vbb.buffer.GetBufferToBind (), vbb.info.bindings, vbb.info.attributes, ib.data.size (), ib.buffer.GetBufferToBind ()),
                                                                 std::move (sp));

    WritableImageResourceP  presentedCopy = WritableImageResource::Create (800, 600);
    SwapchainImageResourceP presented     = SwapchainImageResource::Create (*presentable);


    s.connectionSet.Add (redFillOperation, presented,
                         RG::OutputBinding::Create (0,
                                                    presented->GetFormatProvider (),
                                                    presented->GetFinalLayout (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (redFillOperation, presentedCopy,
                         RG::OutputBinding::Create (1,
                                                    presentedCopy->GetFormatProvider (),
                                                    presentedCopy->GetFinalLayout (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));

    graph.Compile (s);

    BlockingGraphRenderer renderer (s, swapchain);
    window->DoEventLoop (renderer.GetCountLimitedDrawCallback ([&] () -> RG::RenderGraph& { return graph; }, 10));

    CompareImages ("uv", *presentedCopy->images[0]->image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HiddenWindowGoogleTestEnvironment, BasicUniformBufferTest)
{
    DeviceExtra& device        = GetDeviceExtra ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();

    GraphSettings s (device, swapchain.GetImageCount ());
    RenderGraph   graph;

    auto sp = ShaderPipeline::CreateShared (device);
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

    std::shared_ptr<VertexBufferTransferable<Vert>> vbb = VertexBufferTransferable<Vert>::CreateShared (
        device, 4, std::vector<VkFormat> { ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Float }, VK_VERTEX_INPUT_RATE_VERTEX);

    *vbb = std::vector<Vert> {
        { glm::vec2 (-1.f, -1.f), glm::vec2 (0.f, 0.f), 0.1f },
        { glm::vec2 (-1.f, +1.f), glm::vec2 (0.f, 1.f), 0.2f },
        { glm::vec2 (+1.f, +1.f), glm::vec2 (1.f, 1.f), 0.3f },
        { glm::vec2 (+1.f, -1.f), glm::vec2 (1.f, 0.f), 0.6f },
    };
    vbb->Flush ();

    IndexBufferTransferable ib (device, 6);
    ib.data = { 0, 1, 2, 0, 3, 2 };
    ib.Flush ();

    RenderOperationP redFillOperation = RenderOperation::Create (DrawRecordableInfo::CreateShared (1, *vbb, ib), std::move (sp));

    SwapchainImageResourceP presented     = SwapchainImageResource::Create (*presentable);
    WritableImageResourceP  presentedCopy = WritableImageResource::Create (VK_FILTER_LINEAR, 800, 600, 2);
    CPUBufferResourceP      unif          = CPUBufferResource::Create (4);


    s.connectionSet.Add (unif, redFillOperation,
                         UniformInputBinding::Create (0, *unif));

    s.connectionSet.Add (redFillOperation, presented,
                         RG::OutputBinding::Create (2,
                                                    presented->GetFormatProvider (),
                                                    presented->GetFinalLayout (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));


    s.connectionSet.Add (redFillOperation, presentedCopy,
                         RG::OutputBinding::Create (0,
                                                    presentedCopy->GetFormatProvider (),
                                                    presentedCopy->GetFinalLayout (),
                                                    presentedCopy->GetLayerCount (),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    VK_ATTACHMENT_STORE_OP_STORE));

    graph.Compile (s);

    BlockingGraphRenderer renderer (s, swapchain);

    EventObserver obs;
    obs.Observe (renderer.preSubmitEvent, [&] (RenderGraph&, uint32_t frameIndex, uint64_t) {
        float time = 0.5f;
        unif->GetMapping (frameIndex).Copy (time);
    });

    window->DoEventLoop (renderer.GetCountLimitedDrawCallback ([&] () -> RG::RenderGraph& { return graph; }, 10));

    CompareImages ("uvoffset", *presentedCopy->images[0]->image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}
