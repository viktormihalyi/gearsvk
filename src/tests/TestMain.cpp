#include <vulkan/vulkan.h>

#include "Ptr.hpp"
#include "RenderGraph.hpp"
#include "ShaderPipeline.hpp"
#include "Timer.hpp"
#include "Utils.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

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


TEST_F (VulkanTestEnvironment, TestEnvironmentTest)
{
    EXPECT_TRUE (true);
}


TEST_F (VulkanTestEnvironment, DISABLED_RenderGraphConnectionTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetQueue ();

    Graph graph (device, commandPool, 512, 512);

    Resource::Ref depthBuffer    = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool));
    Resource::Ref depthBuffer2   = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool));
    Resource::Ref gbuffer1       = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool));
    Resource::Ref gbuffer2       = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool));
    Resource::Ref gbuffer3       = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool));
    Resource::Ref debugOutput    = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool));
    Resource::Ref lightingBuffer = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool));
    Resource::Ref finalTarget    = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool));

    Operation::Ref depthPass   = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, std::vector<std::filesystem::path> {}));
    Operation::Ref gbufferPass = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, std::vector<std::filesystem::path> {}));
    Operation::Ref debugView   = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, std::vector<std::filesystem::path> {}));
    Operation::Ref move        = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, std::vector<std::filesystem::path> {}));
    Operation::Ref lighting    = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, std::vector<std::filesystem::path> {}));
    Operation::Ref post        = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, std::vector<std::filesystem::path> {}));
    Operation::Ref present     = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, std::vector<std::filesystem::path> {}));

    depthPass.get ().AddOutput (0, depthBuffer);

    gbufferPass.get ().AddInput (0, depthBuffer);
    gbufferPass.get ().AddOutput (0, depthBuffer2);
    gbufferPass.get ().AddOutput (1, gbuffer1);
    gbufferPass.get ().AddOutput (2, gbuffer2);
    gbufferPass.get ().AddOutput (3, gbuffer3);

    debugView.get ().AddInput (0, gbuffer3);
    debugView.get ().AddOutput (0, debugOutput);

    lighting.get ().AddInput (0, depthBuffer);
    lighting.get ().AddInput (1, gbuffer1);
    lighting.get ().AddInput (2, gbuffer2);
    lighting.get ().AddInput (3, gbuffer3);
    lighting.get ().AddOutput (0, lightingBuffer);

    post.get ().AddInput (0, lightingBuffer);
    post.get ().AddOutput (0, finalTarget);

    move.get ().AddInput (0, debugOutput);
    move.get ().AddOutput (0, finalTarget);

    present.get ().AddInput (0, finalTarget);
}


TEST_F (VulkanTestEnvironment, CompileTest)
{
    Device&      device      = GetDevice ();
    CommandPool& commandPool = GetCommandPool ();

    Graph graph (device, commandPool, 2048, 2048);
    graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, std::vector<std::filesystem::path> {
                                                                                                    PROJECT_ROOT / "shaders" / "test.vert",
                                                                                                    PROJECT_ROOT / "shaders" / "test.frag",
                                                                                                }));
}

TEST_F (VulkanTestEnvironment, RenderGraphUseTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetQueue ();

    Graph         graph (device, commandPool, 2048, 2048);
    Resource::Ref presented = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool));
    Resource::Ref green     = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool));
    Resource::Ref red       = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool));

    Operation::Ref dummyPass = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, std::vector<std::filesystem::path> {
                                                                                                                               PROJECT_ROOT / "shaders" / "test.vert",
                                                                                                                               PROJECT_ROOT / "shaders" / "test.frag",
                                                                                                                           }));
    dummyPass.get ().AddInput (0, green);
    dummyPass.get ().AddOutput (0, presented);
    dummyPass.get ().AddOutput (1, red);

    graph.Compile ();

    Utils::TimerLogger obs;
    {
        Utils::TimerScope _ (obs);
        for (uint32_t i = 0; i < 1; ++i) {
            graph.Submit (graphicsQueue);
        }
    }

    vkQueueWaitIdle (GetQueue ());
    vkDeviceWaitIdle (GetDevice ());

    std::thread saveThreads[] = {
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (green.get ()).image.image, "green.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (presented.get ()).image.image, "presented.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (red.get ()).image.image, "red.png"),
    };
    for (auto& t : saveThreads) {
        t.join ();
        std::cout << "saved" << std::endl;
    }
}
