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

    Graph graph (device, commandPool, 1, 512, 512);

    Resource& depthBuffer    = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource& depthBuffer2   = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource& gbuffer1       = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource& gbuffer2       = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource& gbuffer3       = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource& debugOutput    = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource& lightingBuffer = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource& finalTarget    = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));

    Operation& depthPass   = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));
    Operation& gbufferPass = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));
    Operation& debugView   = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));
    Operation& move        = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));
    Operation& lighting    = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));
    Operation& post        = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));
    Operation& present     = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));

    depthPass.AddOutput (0, depthBuffer);

    gbufferPass.AddInput (0, depthBuffer);
    gbufferPass.AddOutput (0, depthBuffer2);
    gbufferPass.AddOutput (1, gbuffer1);
    gbufferPass.AddOutput (2, gbuffer2);
    gbufferPass.AddOutput (3, gbuffer3);

    debugView.AddInput (0, gbuffer3);
    debugView.AddOutput (0, debugOutput);

    lighting.AddInput (0, depthBuffer);
    lighting.AddInput (1, gbuffer1);
    lighting.AddInput (2, gbuffer2);
    lighting.AddInput (3, gbuffer3);
    lighting.AddOutput (0, lightingBuffer);

    post.AddInput (0, lightingBuffer);
    post.AddOutput (0, finalTarget);

    move.AddInput (0, debugOutput);
    move.AddOutput (0, finalTarget);

    present.AddInput (0, finalTarget);
}


TEST_F (VulkanTestEnvironment, CompileTest)
{
    Device&      device      = GetDevice ();
    CommandPool& commandPool = GetCommandPool ();

    Graph graph (device, commandPool, 1, 2048, 2048);
    graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {
                                                                                                       PROJECT_ROOT / "shaders" / "test.vert",
                                                                                                       PROJECT_ROOT / "shaders" / "test.frag",
                                                                                                   }));
}

template<typename DestinationType, typename SourceType>
DestinationType& DynamicRefCast (std::reference_wrapper<SourceType>& source)
{
    return dynamic_cast<DestinationType&> (source.get ());
}


TEST_F (VulkanTestEnvironment, RenderRedImage)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetQueue ();

    Graph graph (device, commandPool, 1, 512, 512);

    Resource& red = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));

    Operation& redFillOperation = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (),
                                                                                  device,
                                                                                  commandPool,
                                                                                  6,
                                                                                  std::vector<std::filesystem::path> {
                                                                                      PROJECT_ROOT / "shaders" / "fullscreenquad.vert",
                                                                                      PROJECT_ROOT / "shaders" / "red.frag",
                                                                                  }));

    graph.AddConnections ({
        {GraphConnection::Type::Output, redFillOperation, 0, red},
    });

    graph.Compile ();

    graph.Submit (graphicsQueue, 0);
    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    CompareImages ("red", *dynamic_cast<ImageResource&> (red).image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (VulkanTestEnvironment, RenderGraphUseTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetQueue ();

    Graph graph (device, commandPool, 4, 512, 512);

    Resource::Ref presented = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
    Resource::Ref green     = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, std::nullopt));
    Resource::Ref red       = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
    Resource::Ref finalImg  = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));

    Operation::Ref dummyPass = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {
                                                                                                                                  PROJECT_ROOT / "shaders" / "test.vert",
                                                                                                                                  PROJECT_ROOT / "shaders" / "test.frag",
                                                                                                                              }));

    Operation::Ref secondPass = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 6, std::vector<std::filesystem::path> {
                                                                                                                                   PROJECT_ROOT / "shaders" / "fullscreenquad.vert",
                                                                                                                                   PROJECT_ROOT / "shaders" / "fullscreenquad.frag",
                                                                                                                               }));
    struct GraphConnection {
        enum class Type {
            Input,
            Output
        };
        Type           type;
        Operation::Ref operation;
        uint32_t       binding;
        Resource::Ref  resource;
    };

    GraphConnection connections[] = {
        {GraphConnection::Type::Input, dummyPass, 0, green},
        {GraphConnection::Type::Output, dummyPass, 0, presented},
        {GraphConnection::Type::Output, dummyPass, 1, red},

        {GraphConnection::Type::Input, secondPass, 0, red},
        {GraphConnection::Type::Output, secondPass, 0, finalImg},
    };

    for (auto c : connections) {
        if (c.type == GraphConnection::Type::Input) {
            c.operation.get ().AddInput (c.binding, c.resource);
        } else {
            c.operation.get ().AddOutput (c.binding, c.resource);
        }
    }

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

    TransitionImageLayout (device, graphicsQueue, commandPool, *DynamicRefCast<ImageResource> (green).image.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *DynamicRefCast<ImageResource> (presented).image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *DynamicRefCast<ImageResource> (red).image.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *DynamicRefCast<ImageResource> (finalImg).image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    std::thread saveThreads[] = {
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (green.get ()).image.image, PROJECT_ROOT / "green.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (presented.get ()).image.image, PROJECT_ROOT / "presented.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (red.get ()).image.image, PROJECT_ROOT / "red.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (finalImg.get ()).image.image, PROJECT_ROOT / "final.png"),
    };
    for (auto& t : saveThreads) {
        t.join ();
        std::cout << "saved" << std::endl;
    }

    ASSERT_TRUE (AreImagesEqual (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (presented.get ()).image.image, PROJECT_ROOT / "black.png"));
}
