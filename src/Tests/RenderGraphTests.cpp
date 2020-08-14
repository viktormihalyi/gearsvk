#include <vulkan/vulkan.h>

#include "FullscreenQuad.hpp"
#include "GraphRenderer.hpp"
#include "Ptr.hpp"
#include "RenderGraph.hpp"
#include "ShaderPipeline.hpp"
#include "Timer.hpp"
#include "UniformBlock.hpp"
#include "Utils.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "glmlib.hpp"

#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include "GoogleTestEnvironment.hpp"
#include "SDF.hpp"


const std::filesystem::path ShadersFolder = PROJECT_ROOT / "src" / "Tests" / "shaders";


using namespace RG;

#include "UniformView.hpp"

TEST_F (HiddenWindowGoogleTestEnvironment, Spirvrross2)
{
    auto sm = ShaderModule::CreateFromGLSLString (GetDevice (), ShaderModule::ShaderKind::Fragment, R"(#version 450

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

TEST_F (HiddenWindowGoogleTestEnvironment, DISABLED_MSDFGEN)
{
    // Image2DTransferable glyphs (GetDevice (), GetGraphicsQueue (), GetCommandPool (), VK_FORMAT_R8G8B8A8_UINT, 16, 16, VK_IMAGE_USAGE_SAMPLED_BIT, 128);

    DeviceExtra& device        = GetDeviceExtra ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    GraphSettings s (device, 3, 512, 512);
    RenderGraph   graph;

    WritableImageResource& red    = graph.CreateResource<WritableImageResource> ();
    ReadOnlyImageResource& glyphs = graph.CreateResource<ReadOnlyImageResource> (VK_FORMAT_R32_SFLOAT, 32, 32, 1, 512);

    auto sp = ShaderPipeline::Create (device);
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

layout (location = 0) in vec2 textureCoords;
layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2DArray sampl;

void main () {
    outColor = vec4 (vec3 (texture (sampl, vec3 (textureCoords, 2)).r), 1.f);
}
    )");

    Operation& redFillOperation = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 6),
                                                                               std::move (sp)));

    graph.CreateOutputConnection (redFillOperation, 0, red);
    graph.CreateInputConnection<ImageInputBinding> (redFillOperation, 0, glyphs);

    graph.Compile (s);


    std::map<char, uint32_t> charToLayerMapping;
    std::vector<float>       asd;
    for (uint32_t i = 0; i < 255; ++i) {
        if (i == 32 || i == 160)
            continue;
        asd = GetGlyphSDF32x32x1 ("C:\\Windows\\Fonts\\arialbd.ttf", i);
        glyphs.CopyLayer (asd, i);
        charToLayerMapping[i] = i;
    }


    graph.Submit (0);
    graph.Submit (1);
    graph.Submit (2);

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    for (uint32_t i = 0; i < 255; ++i) {
        //SaveImageToFileAsync (device, *glyphs.image->imageGPU->image, ReferenceImagesFolder / ("" + std::to_string (i) + "glyphA.png"), i).join ();
    }

    //SaveImageToFileAsync (device, *glyphs.image->imageGPU->image, ReferenceImagesFolder / "glyphA.png", 0).join ();
    //SaveImageToFileAsync (device, *glyphs.image->imageGPU->image, ReferenceImagesFolder / "glyphB.png", 1).join ();
    //SaveImageToFileAsync (device, *glyphs.image->imageGPU->image, ReferenceImagesFolder / "glyphC.png", 2).join ();
    //SaveImageToFileAsync (device, *red.images[0]->image.image, ReferenceImagesFolder / "glyphAout.png").join ();
}


TEST_F (HeadlessGoogleTestEnvironment, CompileTest)
{
    Device&      device      = GetDevice ();
    Queue&       queue       = GetGraphicsQueue ();
    CommandPool& commandPool = GetCommandPool ();

    RenderGraph graph;
    graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3),
                                                 ShaderPipeline::CreateShared (device, std::vector<std::filesystem::path> {
                                                                                           ShadersFolder / "test.vert",
                                                                                           ShadersFolder / "test.frag",
                                                                                       })));
}


TEST_F (HeadlessGoogleTestEnvironment, ShaderCompileTests)
{
    try {
        ShaderModule::CreateFromGLSLString (GetDevice (), ShaderModule::ShaderKind::Vertex, R"(
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

namespace RenderGraphUtils {


RawImageData RenderAndGetImageData (RenderGraph& renderGraph, ImageResource& sw)
{
    renderGraph.Submit (0);

    vkQueueWaitIdle (renderGraph.GetGraphSettings ().GetGrahpicsQueue ());
    vkDeviceWaitIdle (renderGraph.GetGraphSettings ().GetDevice ());

    return RawImageData (renderGraph.GetGraphSettings ().GetDevice (), *sw.GetImages ()[0], 0);
}

} // namespace RenderGraphUtils


TEST_F (HeadlessGoogleTestEnvironment, RenderRedImage)
{
    DeviceExtra& device        = GetDeviceExtra ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    GraphSettings s (device, 3, 512, 512);
    RenderGraph   graph;

    ImageResource& red = graph.CreateResource<WritableImageResource> ();

    auto sp = ShaderPipeline::Create (device);
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

    Operation& redFillOperation = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 6),
                                                                               std::move (sp)));

    graph.CreateOutputConnection (redFillOperation, 0, red);

    graph.Compile (s);

    graph.Submit (0);
    graph.Submit (1);
    graph.Submit (2);

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    CompareImages ("red", *dynamic_cast<WritableImageResource&> (red).images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    CompareImages ("red", *dynamic_cast<WritableImageResource&> (red).images[1]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    CompareImages ("red", *dynamic_cast<WritableImageResource&> (red).images[2]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HeadlessGoogleTestEnvironment, RenderGraphUseTest)
{
    DeviceExtra& device        = GetDeviceExtra ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    GraphSettings s (device, 4, 512, 512);
    RenderGraph   graph;

    WritableImageResource& presented = graph.CreateResource<WritableImageResource> ();
    WritableImageResource& green     = graph.CreateResource<WritableImageResource> ();
    WritableImageResource& red       = graph.CreateResource<WritableImageResource> ();
    WritableImageResource& finalImg  = graph.CreateResource<WritableImageResource> ();

    Operation& dummyPass = graph.CreateOperation<RenderOperation> (DrawRecordableInfo::CreateShared (1, 3),
                                                                   ShaderPipeline::CreateShared (device, std::vector<std::filesystem::path> {
                                                                                                             ShadersFolder / "test.vert",
                                                                                                             ShadersFolder / "test.frag",
                                                                                                         }));

    Operation& secondPass = graph.CreateOperation<RenderOperation> (DrawRecordableInfo::CreateShared (1, 3),
                                                                    ShaderPipeline::CreateShared (device, std::vector<std::filesystem::path> {
                                                                                                              ShadersFolder / "fullscreenquad.vert",
                                                                                                              ShadersFolder / "fullscreenquad.frag",
                                                                                                          }));


    graph.CreateInputConnection<ImageInputBinding> (dummyPass, 0, green);
    graph.CreateOutputConnection (dummyPass, 0, presented);
    graph.CreateOutputConnection (dummyPass, 1, red);

    graph.CreateInputConnection<ImageInputBinding> (secondPass, 0, red);
    graph.CreateOutputConnection (secondPass, 0, finalImg);

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

    RawImageData (device, *green.images[0]->image.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).SaveTo (ReferenceImagesFolder / "green.png");
    RawImageData (device, *presented.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).SaveTo (ReferenceImagesFolder / "presented.png");
    RawImageData (device, *red.images[0]->image.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).SaveTo (ReferenceImagesFolder / "red.png");
    RawImageData (device, *finalImg.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).SaveTo (ReferenceImagesFolder / "final.png");

    ASSERT_TRUE (AreImagesEqual (device, *presented.images[0]->image.image, ReferenceImagesFolder / "black.png"));
}


TEST_F (HiddenWindowGoogleTestEnvironment, SwapchainTest)
{
    DeviceExtra& device        = GetDeviceExtra ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();


    GraphSettings s (device, swapchain);
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

    ImageResource& presentedCopy = graph.CreateResource<WritableImageResource> ();
    ImageResource& presented     = graph.CreateResource<SwapchainImageResource> (swapchain);

    Operation& redFillOperation = graph.CreateOperation<RenderOperation> (DrawRecordableInfo::CreateShared (1, 6), sp);

    graph.CreateOutputConnection (redFillOperation, 0, presented);
    graph.CreateOutputConnection (redFillOperation, 1, presentedCopy);

    graph.Compile (s);

    BlockingGraphRenderer renderer (graph, swapchain);
    window->DoEventLoop (renderer.GetCountLimitedDrawCallback (10));
}


TEST_F (HiddenWindowGoogleTestEnvironment, VertexAndIndexBufferTest)
{
    DeviceExtra&  device        = GetDeviceExtra ();
    CommandPool&  commandPool   = GetCommandPool ();
    Queue&        graphicsQueue = GetGraphicsQueue ();
    Swapchain&    swapchain     = GetSwapchain ();
    GraphSettings s (device, swapchain);
    RenderGraph   graph;

    auto sp = ShaderPipeline::Create (device);
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

    WritableImageResource&  presentedCopy = graph.CreateResource<WritableImageResource> ();
    SwapchainImageResource& presented     = graph.CreateResource<SwapchainImageResource> (swapchain);

    struct Vert {
        glm::vec2 position;
        glm::vec2 uv;
        float     asd;
    };

    VertexBufferTransferable<Vert> vbb (device, 4, {ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Float});
    vbb = std::vector<Vert> {
        {glm::vec2 (-1.f, -1.f), glm::vec2 (0.f, 0.f), 0.1f},
        {glm::vec2 (-1.f, +1.f), glm::vec2 (0.f, 1.f), 0.2f},
        {glm::vec2 (+1.f, +1.f), glm::vec2 (1.f, 1.f), 0.3f},
        {glm::vec2 (+1.f, -1.f), glm::vec2 (1.f, 0.f), 0.6f},
    };
    vbb.Flush ();

    IndexBufferTransferable ib (device, 6);
    ib.data = {0, 1, 2, 0, 3, 2};
    ib.Flush ();

    RenderOperation& redFillOperation = graph.CreateOperation<RenderOperation> (DrawRecordableInfo::CreateShared (1, vbb.data.size (), vbb.buffer.GetBufferToBind (), vbb.info.bindings, vbb.info.attributes, ib.data.size (), ib.buffer.GetBufferToBind ()),
                                                                                std::move (sp));

    graph.CreateOutputConnection (redFillOperation, 0, presented);
    graph.CreateOutputConnection (redFillOperation, 1, presentedCopy);

    graph.Compile (s);

    BlockingGraphRenderer renderer (graph, swapchain);
    window->DoEventLoop (renderer.GetCountLimitedDrawCallback (10));

    CompareImages ("uv", *presentedCopy.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HiddenWindowGoogleTestEnvironment, BasicUniformBufferTest)
{
    DeviceExtra& device        = GetDeviceExtra ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();

    GraphSettings s (device, swapchain);
    RenderGraph   graph;

    auto sp = ShaderPipeline::Create (device);
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

layout (binding = 0) uniform Time {
    float time;
} time;

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

    SwapchainImageResource& presented     = graph.CreateResource<SwapchainImageResource> (swapchain);
    WritableImageResource&  presentedCopy = graph.CreateResource<WritableImageResource> (2);
    UniformBlockResource&   unif          = graph.CreateResource<UniformBlockResource> (4);

    struct Vert {
        glm::vec2 position;
        glm::vec2 uv;
        float     asd;
    };

    VertexBufferTransferable<Vert> vbb (device, 4, {ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Float});
    vbb = std::vector<Vert> {
        {glm::vec2 (-1.f, -1.f), glm::vec2 (0.f, 0.f), 0.1f},
        {glm::vec2 (-1.f, +1.f), glm::vec2 (0.f, 1.f), 0.2f},
        {glm::vec2 (+1.f, +1.f), glm::vec2 (1.f, 1.f), 0.3f},
        {glm::vec2 (+1.f, -1.f), glm::vec2 (1.f, 0.f), 0.6f},
    };
    vbb.Flush ();

    IndexBufferTransferable ib (device, 6);
    ib.data = {0, 1, 2, 0, 3, 2};
    ib.Flush ();

    Operation& redFillOperation = graph.CreateOperation<RenderOperation> (DrawRecordableInfo::CreateShared (1, vbb, ib), std::move (sp));

    graph.CreateInputConnection<UniformInputBinding> (redFillOperation, 0, unif);
    graph.CreateOutputConnection (redFillOperation, 0, presentedCopy);
    graph.CreateOutputConnection (redFillOperation, 2, presented);

    graph.Compile (s);

    BlockingGraphRenderer renderer (graph, swapchain);

    renderer.preSubmitEvent += [&] (uint32_t frameIndex, uint64_t) {
        float time = 0.5f;
        unif.GetMapping (frameIndex).Copy (time);
    };

    window->DoEventLoop (renderer.GetCountLimitedDrawCallback (10));

    CompareImages ("uvoffset", *presentedCopy.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}
