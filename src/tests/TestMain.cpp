#include <vulkan/vulkan.h>

#include "GraphRenderer.hpp"
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

using namespace RenderGraphns;


struct ShaderType {
    uint32_t size;
    uint32_t alignment;
};

static ShaderType vec1 {4, 4};
static ShaderType vec2 {8, 8};
static ShaderType vec3 {12, 16};
static ShaderType vec4 {12, 16};

template<uint32_t SIZE>
static ShaderType vec1Array {4 * SIZE, 32};
template<uint32_t SIZE>
static ShaderType vec2Array {8 * SIZE, 32};
template<uint32_t SIZE>
static ShaderType vec3Array {12 * SIZE, 32};
template<uint32_t SIZE>
static ShaderType vec4Array {16 * SIZE, 32};

class UniformBlock {
public:
    uint32_t fullSize;

    std::vector<std::pair<ShaderType, uint32_t>> variables;

    UniformBlock (const std::vector<ShaderType>& types)
    {
        uint32_t offset = 0;
        for (auto s : types) {
            offset = std::ceil (static_cast<float> (offset) / s.alignment) * s.alignment;
            variables.emplace_back (s, offset);
            offset += s.size;
        }
        fullSize = offset;
    }
};

TEST_F (HeadlessVulkanTestEnvironment, TestEnvironmentTest)
{
    UniformBlock b ({
        vec1,
        vec1,
        vec4,
        vec3,
        vec4Array<3>,
        vec1Array<6>,
    });

    EXPECT_TRUE (true);
}


TEST_F (HeadlessVulkanTestEnvironment, DISABLED_RenderGraphConnectionTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    RenderGraph graph (device, commandPool, GraphSettings (device, graphicsQueue, commandPool, 1, 512, 512));

    Resource& depthBuffer    = graph.AddResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& depthBuffer2   = graph.AddResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& gbuffer1       = graph.AddResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& gbuffer2       = graph.AddResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& gbuffer3       = graph.AddResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& debugOutput    = graph.AddResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& lightingBuffer = graph.AddResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& finalTarget    = graph.AddResource (ImageResource::Create (graph.GetGraphSettings ()));

    Operation& depthPass   = graph.AddOperation (RenderOperation::Create (graph.GetGraphSettings (), DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& gbufferPass = graph.AddOperation (RenderOperation::Create (graph.GetGraphSettings (), DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& debugView   = graph.AddOperation (RenderOperation::Create (graph.GetGraphSettings (), DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& move        = graph.AddOperation (RenderOperation::Create (graph.GetGraphSettings (), DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& lighting    = graph.AddOperation (RenderOperation::Create (graph.GetGraphSettings (), DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& post        = graph.AddOperation (RenderOperation::Create (graph.GetGraphSettings (), DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& present     = graph.AddOperation (RenderOperation::Create (graph.GetGraphSettings (), DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));

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

    RenderGraph graph (device, commandPool, GraphSettings (device, queue, commandPool, 1, 2048, 2048));
    graph.AddOperation (RenderOperation::Create (graph.GetGraphSettings (),
                                                 DrawRecordableInfo::CreateShared (1, 3),
                                                 ShaderPipeline::CreateShared (device, std::vector<std::filesystem::path> {
                                                                                           PROJECT_ROOT / "shaders" / "test.vert",
                                                                                           PROJECT_ROOT / "shaders" / "test.frag",
                                                                                       })));
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

    RenderGraph graph (device, commandPool, GraphSettings (device, graphicsQueue, commandPool, 3, 512, 512));

    Resource& red = graph.AddResource (ImageResource::Create (graph.GetGraphSettings ()));

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

    Operation& redFillOperation = graph.AddOperation (RenderOperation::Create (graph.GetGraphSettings (),
                                                                               DrawRecordableInfo::CreateShared (1, 6),
                                                                               std::move (sp)));

    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 0, red});

    graph.Compile ();

    graph.Submit (0);
    graph.Submit (1);
    graph.Submit (2);

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

    RenderGraph graph (device, commandPool, GraphSettings (device, graphicsQueue, commandPool, 4, 512, 512));

    Resource::Ref presented = graph.CreateResourceTyped<ImageResource> ();
    Resource::Ref green     = graph.CreateResourceTyped<ImageResource> ();
    Resource::Ref red       = graph.CreateResourceTyped<ImageResource> ();
    Resource::Ref finalImg  = graph.CreateResourceTyped<ImageResource> ();

    Operation::Ref dummyPass = graph.AddOperation (RenderOperation::Create (graph.GetGraphSettings (),
                                                                            DrawRecordableInfo::CreateShared (1, 3),
                                                                            ShaderPipeline::CreateShared (device, std::vector<std::filesystem::path> {
                                                                                                                      PROJECT_ROOT / "shaders" / "test.vert",
                                                                                                                      PROJECT_ROOT / "shaders" / "test.frag",
                                                                                                                  })));

    Operation::Ref secondPass = graph.AddOperation (RenderOperation::Create (graph.GetGraphSettings (),
                                                                             DrawRecordableInfo::CreateShared (1, 3),
                                                                             ShaderPipeline::CreateShared (device, std::vector<std::filesystem::path> {
                                                                                                                       PROJECT_ROOT / "shaders" / "fullscreenquad.vert",
                                                                                                                       PROJECT_ROOT / "shaders" / "fullscreenquad.frag",
                                                                                                                   })));


    graph.AddConnection (RenderGraph::InputConnection {dummyPass, 0, green});
    graph.AddConnection (RenderGraph::OutputConnection {dummyPass, 0, presented});
    graph.AddConnection (RenderGraph::OutputConnection {dummyPass, 1, red});

    graph.AddConnection (RenderGraph::InputConnection {secondPass, 0, red});
    graph.AddConnection (RenderGraph::OutputConnection {secondPass, 0, finalImg});

    graph.Compile ();

    Utils::TimerLogger obs;
    {
        Utils::TimerScope _ (obs);
        for (uint32_t i = 0; i < 4; ++i) {
            graph.Submit (i);
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


static void LimitedEventLoop (Window& window, const uint32_t maxRenders, const Window::DrawCallback& callback)
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


    RenderGraph graph (device, commandPool, GraphSettings (device, graphicsQueue, commandPool, swapchain));

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

    Resource& presentedCopy = graph.AddResource (ImageResource::Create (graph.GetGraphSettings ()));
    Resource& presented     = graph.AddResource (SwapchainImageResource::Create (graph.GetGraphSettings (), swapchain));

    Operation& redFillOperation = graph.AddOperation (RenderOperation::Create (graph.GetGraphSettings (),
                                                                               DrawRecordableInfo::CreateShared (1, 6),
                                                                               std::move (sp)));

    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 0, presented});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 1, presentedCopy});

    graph.Compile ();

    BlockingGraphRenderer renderer (graph, swapchain);
    window->DoEventLoop (renderer.GetCountLimitedDrawCallback (10));
}


TEST_F (HiddenWindowVulkanTestEnvironment, VertexAndIndexBufferTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();
    RenderGraph  graph (device, commandPool, GraphSettings (device, graphicsQueue, commandPool, swapchain));

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

    VertexBufferTransferable<Vert> vbb (device, graphicsQueue, commandPool, {ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Float}, 4);
    vbb = std::vector<Vert> {
        {glm::vec2 (-1.f, -1.f), glm::vec2 (0.f, 0.f), 0.1f},
        {glm::vec2 (-1.f, +1.f), glm::vec2 (0.f, 1.f), 0.2f},
        {glm::vec2 (+1.f, +1.f), glm::vec2 (1.f, 1.f), 0.3f},
        {glm::vec2 (+1.f, -1.f), glm::vec2 (1.f, 0.f), 0.6f},
    };
    vbb.Flush ();

    IndexBufferTransferable ib (device, graphicsQueue, commandPool, 6);
    ib.data = {0, 1, 2, 0, 3, 2};
    ib.Flush ();

    RenderOperation& redFillOperation = graph.CreateOperationTyped<RenderOperation> (DrawRecordableInfo::CreateShared (1, vbb.data.size (), vbb.buffer.GetBufferToBind (), vbb.info.bindings, vbb.info.attributes, ib.data.size (), ib.buffer.GetBufferToBind ()),
                                                                                     std::move (sp));

    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 0, presented});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 1, presentedCopy});

    graph.Compile ();

    BlockingGraphRenderer renderer (graph, swapchain);
    window->DoEventLoop (renderer.GetCountLimitedDrawCallback (10));

    CompareImages ("uv", *presentedCopy.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HiddenWindowVulkanTestEnvironment, BasicUniformBufferTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();
    RenderGraph  graph (device, commandPool, GraphSettings (device, graphicsQueue, commandPool, swapchain));

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

    VertexBufferTransferable<Vert> vbb (device, graphicsQueue, commandPool, {ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Float}, 4);
    vbb = std::vector<Vert> {
        {glm::vec2 (-1.f, -1.f), glm::vec2 (0.f, 0.f), 0.1f},
        {glm::vec2 (-1.f, +1.f), glm::vec2 (0.f, 1.f), 0.2f},
        {glm::vec2 (+1.f, +1.f), glm::vec2 (1.f, 1.f), 0.3f},
        {glm::vec2 (+1.f, -1.f), glm::vec2 (1.f, 0.f), 0.6f},
    };
    vbb.Flush ();

    IndexBufferTransferable ib (device, graphicsQueue, commandPool, 6);
    ib.data = {0, 1, 2, 0, 3, 2};
    ib.Flush ();

    Operation& redFillOperation = graph.CreateOperationTyped<RenderOperation> (DrawRecordableInfo::CreateShared (1, vbb, ib), std::move (sp));

    graph.AddConnection (RenderGraph::InputConnection {redFillOperation, 0, unif});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 0, presentedCopy});
    graph.AddConnection (RenderGraph::OutputConnection {redFillOperation, 2, presented});

    graph.Compile ();

    BlockingGraphRenderer renderer (graph, swapchain, [&] (uint32_t frameIndex) {
        float time = 0.5f;
        unif.GetMapping (frameIndex).Copy (time);
    });
    window->DoEventLoop (renderer.GetCountLimitedDrawCallback (10));

    CompareImages ("uvoffset", *presentedCopy.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

/*
#include "Cache.hpp"
#include <utility>

namespace GraphV2 {

class Node;

class OneWayConnection : public std::enable_shared_from_this<OneWayConnection> {
public:
    std::weak_ptr<Node> from;
    std::weak_ptr<Node> to;

    USING_PTR (OneWayConnection);

    OneWayConnection (std::shared_ptr<Node>& from, std::shared_ptr<Node>& to)
        : from (from)
        , to (to)
    {
        ASSERT (IsValidConnection ());
    }

    virtual bool IsValidConnection () const { return true; }
};


class Node : public std::enable_shared_from_this<Node> {
public:
    USING_PTR (Node);
    friend class Graph;

private:
    using NodeList       = std::vector<Node::W>;
    using ConnectionList = std::vector<OneWayConnection::W>;

    ConnectionList  connections;
    Cache<NodeList> pointingHere;
    Cache<NodeList> pointingTo;

    NodeList Node::GetNodesConnectedFromThis ()
    {
        auto thisShared = shared_from_this ();

        NodeList result;
        for (auto& c : connections) {
            if (auto cs = c.lock ()) {
                if (thisShared == cs->from.lock ()) {
                    result.push_back (cs->to);
                }
            }
        }
        return result;
    }

    NodeList Node::GetNodesConnectedToThis ()
    {
        auto thisShared = shared_from_this ();

        NodeList result;
        for (auto& c : connections) {
            if (auto cs = c.lock ()) {
                if (thisShared == cs->to.lock ()) {
                    result.push_back (cs->from);
                }
            }
        }
        return result;
    }


public:
    Node ()
        : pointingHere (std::bind (&Node::GetNodesConnectedToThis, this))
        , pointingTo (std::bind (&Node::GetNodesConnectedFromThis, this))
    {
    }

    virtual ~Node () = default;

    const NodeList& GetInputs () { return pointingHere; }
    const NodeList& GetOutputs () { return pointingTo; }

    virtual bool operator== (const Node& other) const { return false; }
    virtual bool CanConnect (const Node::P& other) const { return true; }

public:
    // either way
    void AddConnection (OneWayConnection::P conn)
    {
        connections.push_back (conn);

        pointingHere.Invalidate ();
        pointingTo.Invalidate ();
    }
};


class Graph {
private:
    std::set<Node::P>             nodes;
    std::set<OneWayConnection::P> connections;

public:
    USING_PTR (Graph);

    template<typename ConnectionType, typename... ARGS>
    std::shared_ptr<ConnectionType> AddConnection (Node::P& from, Node::P& to, ARGS&&... args)
    {
        if (ERROR (!to->CanConnect (from))) {
            return nullptr;
        }

        std::shared_ptr<ConnectionType> asd = std::make_shared<ConnectionType> (from, to, std::forward<ARGS> (args)...);

        if (ERROR (!asd->IsValidConnection ())) {
            return nullptr;
        }

        from->AddConnection (asd);
        to->AddConnection (asd);
        connections.insert (asd);
        return asd;
    }

    template<typename NodeType, typename... ARGS>
    std::shared_ptr<NodeType> AddNode (ARGS&&... args)
    {
        std::shared_ptr<NodeType> asd = std::make_shared<NodeType> (std::forward<ARGS> (args)...);

        for (Node::P n : nodes) {
            if (*n == *asd) {
                return nullptr;
            }
        }

        nodes.insert (asd);
        return asd;
    }
};


class ResourceNode : public Node {
public:
    int i = 2;

    virtual bool CanConnect (const Node::P& other) const
    {
        // cannot connect to itself
        return std::dynamic_pointer_cast<ResourceNode> (other) == nullptr;
    }
};


class OperationNode : public Node {
public:
    int i = 3;

    virtual bool CanConnect (const Node::P& other) const
    {
        // cannot connect to itself
        return std::dynamic_pointer_cast<OperationNode> (other) == nullptr;
    }
};


class InputConnection : public OneWayConnection {
public:
    InputConnection (Node::P& from, Node::P& to, uint32_t)
        : OneWayConnection (from, to)
    {
    }

    virtual bool IsValidConnection () const override
    {
        return std::dynamic_pointer_cast<ResourceNode> (from.lock ()) != nullptr &&
               std::dynamic_pointer_cast<OperationNode> (to.lock ()) != nullptr;
    }
};


class OutputConnection : public OneWayConnection {
public:
    OutputConnection (Node::P& from, Node::P& to, uint32_t)
        : OneWayConnection (from, to)
    {
    }

    virtual bool IsValidConnection () const override
    {
        return std::dynamic_pointer_cast<OperationNode> (from.lock ()) != nullptr &&
               std::dynamic_pointer_cast<ResourceNode> (to.lock ()) != nullptr;
    }
};

} // namespace GraphV2


TEST_F (HiddenWindowVulkanTestEnvironment, graphtestttt)
{
    using namespace GraphV2;

    GraphV2::Graph g;

    Node::P r00 = g.AddNode<ResourceNode> ();
    Node::P r01 = g.AddNode<ResourceNode> ();
    Node::P r02 = g.AddNode<ResourceNode> ();
    Node::P r03 = g.AddNode<ResourceNode> ();
    Node::P r04 = g.AddNode<ResourceNode> ();

    Node::P n1 = g.AddNode<OperationNode> ();

    g.AddConnection<InputConnection> (r00, n1, 1);
    g.AddConnection<InputConnection> (r01, n1, 1);
    g.AddConnection<InputConnection> (r02, n1, 1);
    g.AddConnection<InputConnection> (r03, n1, 1);
    g.AddConnection<InputConnection> (r04, n1, 1);

    ASSERT_EQ (5, n1->GetInputs ().size ());
}
*/
