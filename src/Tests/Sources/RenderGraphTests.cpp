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

    std::shared_ptr<RG::ImageResource> red = std::make_unique<RG::WritableImageResource> (512, 512);

    RG::GraphSettings s (GetDeviceExtra (), 3);
    s.connectionSet.Add (redFillOperation, red,
                         std::make_unique<RG::OutputBinding> (0,
                                                              red->GetFormatProvider (),
                                                              red->GetFinalLayout (),
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                              VK_ATTACHMENT_STORE_OP_STORE));

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

    s.connectionSet.Add (redFillOperation, presented,
                         std::make_unique<RG::OutputBinding> (0,
                                                              presented->GetFormatProvider (),
                                                              presented->GetFinalLayout (),
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                              VK_ATTACHMENT_STORE_OP_STORE));

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

    std::shared_ptr<RG::ImageResource> red = std::make_unique<RG::WritableImageResource> (512, 512);

    RG::GraphSettings s (GetDeviceExtra (), 3);
    s.connectionSet.Add (redFillOperation, red,
                         std::make_unique<RG::OutputBinding> (0,
                                                              red->GetFormatProvider (),
                                                              red->GetFinalLayout (),
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                              VK_ATTACHMENT_STORE_OP_STORE));

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

    s.connectionSet.Add (redFillOperation, red,
                         std::make_unique<RG::OutputBinding> (0,
                                                              red->GetFormatProvider (),
                                                              red->GetFinalLayout (),
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                              VK_ATTACHMENT_STORE_OP_STORE));


    s.connectionSet.Add (red, transfer,
                         std::make_unique<RG::ImageInputBinding> (0, *red));

    s.connectionSet.Add (transfer, duplicate,
                         std::make_unique<RG::OutputBinding> (0,
                                                              duplicate->GetFormatProvider (),
                                                              duplicate->GetFinalLayout (),
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                              VK_ATTACHMENT_STORE_OP_STORE));


    RG::RenderGraph graph;
    graph.Compile (std::move (s));
    graph.Submit (0);

    env->Wait ();

    CompareImages ("red", *duplicate->GetImages ()[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}


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


    RG::ConnectionSet connectionSet;
    connectionSet.Add (green, dummyPass, std::make_unique<RG::ImageInputBinding> (0, *green));
    connectionSet.Add (red, secondPass, std::make_unique<RG::ImageInputBinding> (0, *red));

    connectionSet.Add (dummyPass, presented,
                       std::make_unique<RG::OutputBinding> (0,
                                                            presented->GetFormatProvider (),
                                                            presented->GetFinalLayout (),
                                                            VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                            VK_ATTACHMENT_STORE_OP_STORE));

    connectionSet.Add (dummyPass, red,
                       std::make_unique<RG::OutputBinding> (1,
                                                            red->GetFormatProvider (),
                                                            red->GetFinalLayout (),
                                                            VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                            VK_ATTACHMENT_STORE_OP_STORE));


    connectionSet.Add (secondPass, finalImg,
                       std::make_unique<RG::OutputBinding> (0,
                                                            finalImg->GetFormatProvider (),
                                                            finalImg->GetFinalLayout (),
                                                            VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                            VK_ATTACHMENT_STORE_OP_STORE));

    RG::RenderGraph graph;
    graph.Compile (RG::GraphSettings (GetDeviceExtra (), std::move (connectionSet), 4));

    for (uint32_t i = 0; i < 1; ++i) {
        graph.Submit (i);
    }

    env->Wait ();

    ASSERT_TRUE (GVK::ImageData (GetDeviceExtra (), *presented->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) == GVK::ImageData (ReferenceImagesFolder / "black.png"));
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

    s.connectionSet.Add (firstPass, presented,
                         std::make_unique<RG::OutputBinding> (0,
                                                              presented->GetFormatProvider (),
                                                              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                              presented->GetFinalLayout (),
                                                              1,
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                              VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (secondPass, presented,
                         std::make_unique<RG::OutputBinding> (0,
                                                              presented->GetFormatProvider (),
                                                              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                              presented->GetFinalLayout (),
                                                              1,
                                                              VK_ATTACHMENT_LOAD_OP_LOAD,
                                                              VK_ATTACHMENT_STORE_OP_STORE));


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

    std::shared_ptr<RG::ImageResource> presentedCopy = std::make_unique<RG::WritableImageResource> (800, 600);
    std::shared_ptr<RG::ImageResource> presented     = std::make_unique<RG::SwapchainImageResource> (*presentable);

    s.connectionSet.Add (redFillOperation, presented,
                         std::make_unique<RG::OutputBinding> (0,
                                                              presented->GetFormatProvider (),
                                                              presented->GetFinalLayout (),
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                              VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (redFillOperation, presentedCopy,
                         std::make_unique<RG::OutputBinding> (1,
                                                              presentedCopy->GetFormatProvider (),
                                                              presentedCopy->GetFinalLayout (),
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                              VK_ATTACHMENT_STORE_OP_STORE));
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


    s.connectionSet.Add (redFillOperation, presented,
                         std::make_unique<RG::OutputBinding> (0,
                                                              presented->GetFormatProvider (),
                                                              presented->GetFinalLayout (),
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                              VK_ATTACHMENT_STORE_OP_STORE));

    s.connectionSet.Add (redFillOperation, presentedCopy,
                         std::make_unique<RG::OutputBinding> (1,
                                                              presentedCopy->GetFormatProvider (),
                                                              presentedCopy->GetFinalLayout (),
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                              VK_ATTACHMENT_STORE_OP_STORE));

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


    s.connectionSet.Add (unif, redFillOperation,
                         std::make_unique<RG::UniformInputBinding> (0, *unif));

    s.connectionSet.Add (redFillOperation, presented,
                         std::make_unique<RG::OutputBinding> (2,
                                                              presented->GetFormatProvider (),
                                                              presented->GetFinalLayout (),
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                              VK_ATTACHMENT_STORE_OP_STORE));


    s.connectionSet.Add (redFillOperation, presentedCopy,
                         std::make_unique<RG::OutputBinding> (0,
                                                              presentedCopy->GetFormatProvider (),
                                                              presentedCopy->GetFinalLayout (),
                                                              presentedCopy->GetLayerCount (),
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                              VK_ATTACHMENT_STORE_OP_STORE));

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

#if 0

TEST_F (RenderGraphAbstractionTest, XorShiftRNG)
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
    outColor = texture (randomTextureIn[randomTextureIndex], textureCoords);
}
    )";

    const std::string fragSrc2 = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding = 2) uniform CommonUniforms {
    uint 123;
};

layout (binding = 8) uniform sampler2D randomTextureIn[5];

layout (location = 0) out vec4 randomTextureOut[5];

void main () {
    outColor = vec4 (1, 0, 0, 0.5);
}
    )";

    std::shared_ptr<RG::SingleWritableImageResource> renderTarget = std::make_shared<RG::SingleWritableImageResource> (VK_FILTER_LINEAR, 512, 512, 1, VK_FORMAT_R8G8B8A8_SRGB);

    std::shared_ptr<RG::WritableImageResource> renderTarget = std::make_shared<RG::WritableImageResource> (VK_FILTER_LINEAR, 512, 512, 1, VK_FORMAT_R8G8B8A8_SRGB);


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

    RG::GraphSettings s;
    s.framesInFlight = 1;
    s.device         = env->deviceExtra.get ();

    s.connectionSet.Add (renderOp, renderTarget,
                         std::make_unique<RG::OutputBinding> (0,
                                                              renderTarget->GetFormatProvider (),
                                                              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                              renderTarget->GetFinalLayout (),
                                                              1,
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                              VK_ATTACHMENT_STORE_OP_STORE));


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



#endif