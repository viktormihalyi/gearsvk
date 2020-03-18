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

    Resource::Ref depthBuffer    = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource::Ref depthBuffer2   = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource::Ref gbuffer1       = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource::Ref gbuffer2       = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource::Ref gbuffer3       = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource::Ref debugOutput    = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource::Ref lightingBuffer = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));
    Resource::Ref finalTarget    = graph.CreateResource (ImageResource::Create (graph.GetGraphInfo (), device, graphicsQueue, commandPool, std::nullopt, std::nullopt));

    Operation::Ref depthPass   = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));
    Operation::Ref gbufferPass = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));
    Operation::Ref debugView   = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));
    Operation::Ref move        = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));
    Operation::Ref lighting    = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));
    Operation::Ref post        = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));
    Operation::Ref present     = graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {}));

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
    graph.CreateOperation (RenderOperation::Create (graph.GetGraphInfo (), device, commandPool, 3, std::vector<std::filesystem::path> {
                                                                                                       PROJECT_ROOT / "shaders" / "test.vert",
                                                                                                       PROJECT_ROOT / "shaders" / "test.frag",
                                                                                                   }));
}

template<typename DestinationType, typename SourceType>
std::reference_wrapper<DestinationType> DynamicRefCast (std::reference_wrapper<SourceType>& source)
{
    return std::reference_wrapper<DestinationType> (dynamic_cast<DestinationType&> (source.get ()));
}


TEST_F (VulkanTestEnvironment, RenderGraphUseTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetQueue ();

    Graph graph (device, commandPool, 512, 512);

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
        for (uint32_t i = 0; i < 1; ++i) {
            graph.Submit (graphicsQueue);
        }
    }

    vkQueueWaitIdle (GetQueue ());
    vkDeviceWaitIdle (GetDevice ());

    TransitionImageLayout (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (green.get ()).image.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (presented.get ()).image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (red.get ()).image.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *dynamic_cast<ImageResource&> (finalImg.get ()).image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

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
