#include <vulkan/vulkan.h>

#include "Ptr.hpp"
#include "RenderGraph.hpp"
#include "ShaderPipeline.hpp"
#include "Timer.hpp"
#include "Utils.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include <glm/glm.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include "VulkanTestEnvironment.hpp"


int main (int argc, char** argv)
{
    ::testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}

using namespace RenderGraph;


TEST_F (HeadlessVulkanTestEnvironment, TestEnvironmentTest)
{
    EXPECT_TRUE (true);
}


TEST_F (HeadlessVulkanTestEnvironment, DISABLED_RenderGraphConnectionTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    Graph graph (device, commandPool, GraphSettings (device, graphicsQueue, commandPool, 1, 512, 512));

    Resource& depthBuffer    = graph.CreateResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& depthBuffer2   = graph.CreateResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& gbuffer1       = graph.CreateResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& gbuffer2       = graph.CreateResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& gbuffer3       = graph.CreateResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& debugOutput    = graph.CreateResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& lightingBuffer = graph.CreateResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& finalTarget    = graph.CreateResource (ImageResource::Create (graph.GetGraphSettings ()));

    Operation& depthPass   = graph.CreateOperation (RenderOperation::Create (graph.GetGraphSettings (), RenderOperationSettings (1, 3), std::vector<std::filesystem::path> {}));
    Operation& gbufferPass = graph.CreateOperation (RenderOperation::Create (graph.GetGraphSettings (), RenderOperationSettings (1, 3), std::vector<std::filesystem::path> {}));
    Operation& debugView   = graph.CreateOperation (RenderOperation::Create (graph.GetGraphSettings (), RenderOperationSettings (1, 3), std::vector<std::filesystem::path> {}));
    Operation& move        = graph.CreateOperation (RenderOperation::Create (graph.GetGraphSettings (), RenderOperationSettings (1, 3), std::vector<std::filesystem::path> {}));
    Operation& lighting    = graph.CreateOperation (RenderOperation::Create (graph.GetGraphSettings (), RenderOperationSettings (1, 3), std::vector<std::filesystem::path> {}));
    Operation& post        = graph.CreateOperation (RenderOperation::Create (graph.GetGraphSettings (), RenderOperationSettings (1, 3), std::vector<std::filesystem::path> {}));
    Operation& present     = graph.CreateOperation (RenderOperation::Create (graph.GetGraphSettings (), RenderOperationSettings (1, 3), std::vector<std::filesystem::path> {}));

    // depthPass.AddOutput (0, depthBuffer);
    //
    // gbufferPass.AddInput (0, depthBuffer);
    // gbufferPass.AddOutput (0, depthBuffer2);
    // gbufferPass.AddOutput (1, gbuffer1);
    // gbufferPass.AddOutput (2, gbuffer2);
    // gbufferPass.AddOutput (3, gbuffer3);
    //
    // debugView.AddInput (0, gbuffer3);
    // debugView.AddOutput (0, debugOutput);
    //
    // lighting.AddInput (0, depthBuffer);
    // lighting.AddInput (1, gbuffer1);
    // lighting.AddInput (2, gbuffer2);
    // lighting.AddInput (3, gbuffer3);
    // lighting.AddOutput (0, lightingBuffer);
    //
    // post.AddInput (0, lightingBuffer);
    // post.AddOutput (0, finalTarget);
    //
    // move.AddInput (0, debugOutput);
    // move.AddOutput (0, finalTarget);
    //
    // present.AddInput (0, finalTarget);
}


TEST_F (HeadlessVulkanTestEnvironment, CompileTest)
{
    Device&      device      = GetDevice ();
    Queue&       queue       = GetGraphicsQueue ();
    CommandPool& commandPool = GetCommandPool ();

    Graph graph (device, commandPool, GraphSettings (device, queue, commandPool, 1, 2048, 2048));
    graph.CreateOperation (RenderOperation::Create (graph.GetGraphSettings (),
                                                    RenderOperationSettings (1, 3),
                                                    std::vector<std::filesystem::path> {
                                                        PROJECT_ROOT / "shaders" / "test.vert",
                                                        PROJECT_ROOT / "shaders" / "test.frag",
                                                    }));
}

template<typename DestinationType, typename SourceType>
DestinationType& DynamicRefCast (std::reference_wrapper<SourceType>& source)
{
    return dynamic_cast<DestinationType&> (source.get ());
}


TEST_F (HeadlessVulkanTestEnvironment, RenderRedImage)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    Graph graph (device, commandPool, GraphSettings (device, graphicsQueue, commandPool, 3, 512, 512));

    Resource& red = graph.CreateResource (ImageResource::Create (graph.GetGraphSettings ()));

    auto sp = ShaderPipeline::Create (device);
    sp->SetVertexShader (R"(
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

    sp->SetFragmentShader (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (1, 0, 0, 1);
}
    )");

    Operation& redFillOperation = graph.CreateOperation (RenderOperation::Create (graph.GetGraphSettings (),
                                                                                  RenderOperationSettings (1, 6),
                                                                                  std::move (sp)));

    graph.AddConnection (Graph::OutputConnection {redFillOperation, 0, red});

    graph.Compile ();

    graph.Submit (graphicsQueue, 0);
    graph.Submit (graphicsQueue, 1);
    graph.Submit (graphicsQueue, 2);

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    CompareImages ("red", *dynamic_cast<ImageResource&> (red).images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    CompareImages ("red", *dynamic_cast<ImageResource&> (red).images[1]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    CompareImages ("red", *dynamic_cast<ImageResource&> (red).images[2]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HeadlessVulkanTestEnvironment, RenderGraphUseTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    Graph graph (device, commandPool, GraphSettings (device, graphicsQueue, commandPool, 4, 512, 512));

    Resource::Ref presented = graph.CreateResourceTyped<ImageResource> ();
    Resource::Ref green     = graph.CreateResourceTyped<ImageResource> ();
    Resource::Ref red       = graph.CreateResourceTyped<ImageResource> ();
    Resource::Ref finalImg  = graph.CreateResourceTyped<ImageResource> ();

    Operation::Ref dummyPass = graph.CreateOperation (RenderOperation::Create (graph.GetGraphSettings (),
                                                                               RenderOperationSettings (1, 3),
                                                                               std::vector<std::filesystem::path> {
                                                                                   PROJECT_ROOT / "shaders" / "test.vert",
                                                                                   PROJECT_ROOT / "shaders" / "test.frag",
                                                                               }));

    Operation::Ref secondPass = graph.CreateOperation (RenderOperation::Create (graph.GetGraphSettings (),
                                                                                RenderOperationSettings (1, 3),
                                                                                std::vector<std::filesystem::path> {
                                                                                    PROJECT_ROOT / "shaders" / "fullscreenquad.vert",
                                                                                    PROJECT_ROOT / "shaders" / "fullscreenquad.frag",
                                                                                }));


    graph.AddConnection (Graph::InputConnection {dummyPass, 0, green});
    graph.AddConnection (Graph::OutputConnection {dummyPass, 0, presented});
    graph.AddConnection (Graph::OutputConnection {dummyPass, 1, red});

    graph.AddConnection (Graph::InputConnection {secondPass, 0, red});
    graph.AddConnection (Graph::OutputConnection {secondPass, 0, finalImg});

    graph.Compile ();

    Utils::TimerLogger obs;
    {
        Utils::TimerScope _ (obs);
        for (uint32_t i = 0; i < 4; ++i) {
            graph.Submit (graphicsQueue, i);
        }
    }

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    TransitionImageLayout (device, graphicsQueue, commandPool, *DynamicRefCast<ImageResource> (green).images[0]->image.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *DynamicRefCast<ImageResource> (presented).images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *DynamicRefCast<ImageResource> (red).images[0]->image.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *DynamicRefCast<ImageResource> (finalImg).images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    std::thread saveThreads[] = {
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (green.get ()).images[0]->image.image, PROJECT_ROOT / "green.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (presented.get ()).images[0]->image.image, PROJECT_ROOT / "presented.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (red.get ()).images[0]->image.image, PROJECT_ROOT / "red.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (finalImg.get ()).images[0]->image.image, PROJECT_ROOT / "final.png"),
    };
    for (auto& t : saveThreads) {
        t.join ();
        std::cout << "saved" << std::endl;
    }

    ASSERT_TRUE (AreImagesEqual (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (presented.get ()).images[0]->image.image, PROJECT_ROOT / "black.png"));
}


static void LimitedEventLoop (WindowBase& window, const uint32_t maxRenders, const WindowBase::DrawCallback& callback)
{
    uint32_t renderCount = 0;

    bool dummy = false;

    window.DoEventLoop ([&] (bool& stopFlag) {
        callback (dummy);

        renderCount++;
        if (renderCount >= maxRenders) {
            stopFlag = true;
        }
    });
}


TEST_F (HiddenWindowVulkanTestEnvironment, SwapchainTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();

    Graph graph (device, commandPool, GraphSettings (device, graphicsQueue, commandPool, swapchain));

    auto sp = ShaderPipeline::Create (device);
    sp->SetVertexShader (R"(
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

    sp->SetFragmentShader (R"(
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

    Resource& presentedCopy = graph.CreateResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& presented     = graph.CreateResource (SwapchainImageResource::Create (graph.GetGraphSettings (), swapchain));

    Operation& redFillOperation = graph.CreateOperation (RenderOperation::Create (graph.GetGraphSettings (),
                                                                                  RenderOperationSettings (1, 6),
                                                                                  std::move (sp)));

    Operation& presentOp = graph.CreateOperation (PresentOperation::Create (graph.GetGraphSettings (), swapchain, std::vector<VkSemaphore> {}));

    graph.AddConnection (Graph::OutputConnection {redFillOperation, 0, presented});
    graph.AddConnection (Graph::OutputConnection {redFillOperation, 1, presentedCopy});

    graph.Compile ();

    Semaphore s (device);

    std::chrono::time_point<std::chrono::high_resolution_clock> lastDrawTime = std::chrono::high_resolution_clock::now ();

    LimitedEventLoop (*window, 10, [&] (bool& stopFlag) {
        uint32_t imageIndex = 0;
        vkAcquireNextImageKHR (device, swapchain, UINT64_MAX, s, VK_NULL_HANDLE, &imageIndex);
        graph.Submit (graphicsQueue, imageIndex, {s});

        vkQueueWaitIdle (graphicsQueue);
        vkDeviceWaitIdle (device);
    });

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);
}


TEST_F (HiddenWindowVulkanTestEnvironment, VertexAndIndexBufferTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();
    Graph        graph (device, commandPool, GraphSettings (device, graphicsQueue, commandPool, swapchain));

    auto sp = ShaderPipeline::Create (device);
    sp->SetVertexShader (R"(
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

    sp->SetFragmentShader (R"(
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

    ImageResource&          presentedCopy = graph.CreateResourceTyped<ImageResource> ();
    SwapchainImageResource& presented     = graph.CreateResourceTyped<SwapchainImageResource> (swapchain);

    struct Vert {
        glm::vec2 position;
        glm::vec2 uv;
        float     asd;
    };

    TypedTransferableVertexBuffer<Vert> vbb (device, graphicsQueue, commandPool, {ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Float}, 4);
    vbb = std::vector<Vert> {
        {glm::vec2 (-1.f, -1.f), glm::vec2 (0.f, 0.f), 0.1f},
        {glm::vec2 (-1.f, +1.f), glm::vec2 (0.f, 1.f), 0.2f},
        {glm::vec2 (+1.f, +1.f), glm::vec2 (1.f, 1.f), 0.3f},
        {glm::vec2 (+1.f, -1.f), glm::vec2 (1.f, 0.f), 0.6f},
    };
    vbb.Flush ();

    TransferableIndexBuffer ib (device, graphicsQueue, commandPool, 6);
    ib.data = {0, 1, 2, 0, 3, 2};
    ib.Flush ();

    RenderOperation& redFillOperation = graph.CreateOperationTyped<RenderOperation> (RenderOperationSettings (1, vbb.data.size (), vbb.buffer.GetBufferToBind (), vbb.info.bindings, vbb.info.attributes, ib.data.size (), ib.buffer.GetBufferToBind ()),
                                                                                     std::move (sp));

    PresentOperation& presentOp = graph.CreateOperationTyped<PresentOperation> (swapchain, std::vector<VkSemaphore> {});

    graph.AddConnection (Graph::OutputConnection {redFillOperation, 0, presented});
    graph.AddConnection (Graph::OutputConnection {redFillOperation, 1, presentedCopy});

    graph.Compile ();

    Semaphore s (device);

    LimitedEventLoop (*window, 10, [&] (bool&) {
        uint32_t imageIndex = 0;
        vkAcquireNextImageKHR (device, swapchain, UINT64_MAX, s, VK_NULL_HANDLE, &imageIndex);
        graph.Submit (graphicsQueue, imageIndex, {s});

        vkQueueWaitIdle (graphicsQueue);
        vkDeviceWaitIdle (device);
    });

    CompareImages ("uv", *presentedCopy.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HiddenWindowVulkanTestEnvironment, BasicUniformBufferTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();
    Graph        graph (device, commandPool, GraphSettings (device, graphicsQueue, commandPool, swapchain));

    auto sp = ShaderPipeline::Create (device);
    sp->SetVertexShader (R"(
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

    sp->SetFragmentShader (R"(
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

    SwapchainImageResource& presented     = graph.CreateResourceTyped<SwapchainImageResource> (swapchain);
    ImageResource&          presentedCopy = graph.CreateResourceTyped<ImageResource> (2);
    UniformBlockResource&   unif          = graph.CreateResourceTyped<UniformBlockResource> (4);

    struct Vert {
        glm::vec2 position;
        glm::vec2 uv;
        float     asd;
    };

    TypedTransferableVertexBuffer<Vert> vbb (device, graphicsQueue, commandPool, {ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Float}, 4);
    vbb = std::vector<Vert> {
        {glm::vec2 (-1.f, -1.f), glm::vec2 (0.f, 0.f), 0.1f},
        {glm::vec2 (-1.f, +1.f), glm::vec2 (0.f, 1.f), 0.2f},
        {glm::vec2 (+1.f, +1.f), glm::vec2 (1.f, 1.f), 0.3f},
        {glm::vec2 (+1.f, -1.f), glm::vec2 (1.f, 0.f), 0.6f},
    };
    vbb.Flush ();

    TransferableIndexBuffer ib (device, graphicsQueue, commandPool, 6);
    ib.data = {0, 1, 2, 0, 3, 2};
    ib.Flush ();

    Operation& redFillOperation = graph.CreateOperationTyped<RenderOperation> (RenderOperationSettings (1, vbb, ib), std::move (sp));
    Operation& presentOp        = graph.CreateOperationTyped<PresentOperation> (swapchain);

    graph.AddConnection (Graph::InputConnection {redFillOperation, 0, unif});
    graph.AddConnection (Graph::OutputConnection {redFillOperation, 0, presentedCopy});
    graph.AddConnection (Graph::OutputConnection {redFillOperation, 2, presented});

    graph.Compile ();

    Semaphore s (device);

    LimitedEventLoop (*window, 10, [&] (bool&) {
        uint32_t imageIndex = 0;
        vkAcquireNextImageKHR (device, swapchain, UINT64_MAX, s, VK_NULL_HANDLE, &imageIndex);

        float time = 0.5f;
        unif.GetMapping (imageIndex).Copy (time);

        graph.Submit (graphicsQueue, imageIndex, {s});

        vkQueueWaitIdle (graphicsQueue);
        vkDeviceWaitIdle (device);
    });

    CompareImages ("uvoffset", *presentedCopy.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}
