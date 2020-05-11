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


int main (int argc, char** argv)
{
    ::testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}


const std::filesystem::path ShadersFolder = PROJECT_ROOT / "src" / "Tests" / "shaders";


using namespace RG;


TEST_F (EmptyTestEnvironment, UniformBlockTest)
{
    using namespace ST;

    {
        ShaderStruct b ({
            {"f", ST::vec1},
            {"m", ST::mat4},
        });
        EXPECT_EQ (80, b.GetFullSize ());
        EXPECT_EQ (0, b.GetOffset ("f"));
        EXPECT_EQ (16, b.GetOffset ("m"));
    }

    {
        ShaderStruct b ({
            {"m", ST::mat4},
            {"f", ST::vec1},
        });
        EXPECT_EQ (68, b.GetFullSize ());
        EXPECT_EQ (64, b.GetOffset ("f"));
        EXPECT_EQ (0, b.GetOffset ("m"));
    }

    {
        ShaderStruct b ({
            {"f", ST::vec1},
            {"m", ST::mat4},
            {"f2", ST::vec1},
        });
        EXPECT_EQ (80, b.GetOffset ("f2"));
    }

    {
        ShaderStruct b ({
            {"asd", vec1},
            {"4635", mat4},
            {"fd", vec4},
            {"f4", vec3},
            {"865", vec4Array<3>},
            {"23", vec1Array<6>},
        });

        ASSERT (b.GetOffset ("4635") == 16);
    }

    EXPECT_TRUE (true);
}


TEST_F (HeadlessGoogleTestEnvironment, DISABLED_RenderGraphConnectionTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    RenderGraph graph (device, commandPool);

    Resource& depthBuffer    = graph.AddResource (WritableImageResource::Create ());
    Resource& depthBuffer2   = graph.AddResource (WritableImageResource::Create ());
    Resource& gbuffer1       = graph.AddResource (WritableImageResource::Create ());
    Resource& gbuffer2       = graph.AddResource (WritableImageResource::Create ());
    Resource& gbuffer3       = graph.AddResource (WritableImageResource::Create ());
    Resource& debugOutput    = graph.AddResource (WritableImageResource::Create ());
    Resource& lightingBuffer = graph.AddResource (WritableImageResource::Create ());
    Resource& finalTarget    = graph.AddResource (WritableImageResource::Create ());

    Operation& depthPass   = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& gbufferPass = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& debugView   = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& move        = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& lighting    = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& post        = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& present     = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));

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


TEST_F (HeadlessGoogleTestEnvironment, CompileTest)
{
    Device&      device      = GetDevice ();
    Queue&       queue       = GetGraphicsQueue ();
    CommandPool& commandPool = GetCommandPool ();

    RenderGraph graph (device, commandPool);
    graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3),
                                                 ShaderPipeline::CreateShared (device, std::vector<std::filesystem::path> {
                                                                                           ShadersFolder / "test.vert",
                                                                                           ShadersFolder / "test.frag",
                                                                                       })));
}

template<typename DestinationType, typename SourceType>
DestinationType& DynamicRefCast (std::reference_wrapper<SourceType>& source)
{
    return dynamic_cast<DestinationType&> (source.get ());
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


TEST_F (HeadlessGoogleTestEnvironment, RenderRedImage)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    GraphSettings s (device, graphicsQueue, commandPool, 3, 512, 512);
    RenderGraph   graph (device, commandPool);

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
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    GraphSettings s (device, graphicsQueue, commandPool, 4, 512, 512);
    RenderGraph   graph (device, commandPool);

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

    TransitionImageLayout (device, graphicsQueue, commandPool, *green.images[0]->image.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *presented.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *red.images[0]->image.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *finalImg.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    std::thread saveThreads[] = {
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *green.images[0]->image.image, ReferenceImagesFolder / "green.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *presented.images[0]->image.image, ReferenceImagesFolder / "presented.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *red.images[0]->image.image, ReferenceImagesFolder / "red.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *finalImg.images[0]->image.image, ReferenceImagesFolder / "final.png"),
    };
    for (auto& t : saveThreads) {
        t.join ();
        std::cout << "saved" << std::endl;
    }

    ASSERT_TRUE (AreImagesEqual (device, graphicsQueue, commandPool, *presented.images[0]->image.image, ReferenceImagesFolder / "black.png"));
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


TEST_F (HiddenWindowGoogleTestEnvironment, SwapchainTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();


    GraphSettings s (device, graphicsQueue, commandPool, swapchain);
    RenderGraph   graph (device, commandPool);

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
    Device&       device        = GetDevice ();
    CommandPool&  commandPool   = GetCommandPool ();
    Queue&        graphicsQueue = GetGraphicsQueue ();
    Swapchain&    swapchain     = GetSwapchain ();
    GraphSettings s (device, graphicsQueue, commandPool, swapchain);
    RenderGraph   graph (device, commandPool);

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

    VertexBufferTransferable<Vert> vbb (device, graphicsQueue, commandPool, 4, {ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Float});
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
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();

    GraphSettings s (device, graphicsQueue, commandPool, swapchain);
    RenderGraph   graph (device, commandPool);

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

    VertexBufferTransferable<Vert> vbb (device, graphicsQueue, commandPool, 4, {ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Float});
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
    void CreateOutputConnection (OneWayConnection::P conn)
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
    std::shared_ptr<ConnectionType> CreateOutputConnection (Node::P& from, Node::P& to, ARGS&&... args)
    {
        if (ERROR (!to->CanConnect (from))) {
            return nullptr;
        }

        std::shared_ptr<ConnectionType> asd = std::make_shared<ConnectionType> (from, to, std::forward<ARGS> (args)...);

        if (ERROR (!asd->IsValidConnection ())) {
            return nullptr;
        }

        from->CreateOutputConnection (asd);
        to->CreateOutputConnection (asd);
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


TEST_F (HiddenWindowGoogleTestEnvironment, graphtestttt)
{
    using namespace GraphV2;

    GraphV2::Graph g;

    Node::P r00 = g.AddNode<ResourceNode> ();
    Node::P r01 = g.AddNode<ResourceNode> ();
    Node::P r02 = g.AddNode<ResourceNode> ();
    Node::P r03 = g.AddNode<ResourceNode> ();
    Node::P r04 = g.AddNode<ResourceNode> ();

    Node::P n1 = g.AddNode<OperationNode> ();

    g.CreateOutputConnection<InputConnection> (r00, n1, 1);
    g.CreateOutputConnection<InputConnection> (r01, n1, 1);
    g.CreateOutputConnection<InputConnection> (r02, n1, 1);
    g.CreateOutputConnection<InputConnection> (r03, n1, 1);
    g.CreateOutputConnection<InputConnection> (r04, n1, 1);

    ASSERT_EQ (5, n1->GetInputs ().size ());
}
*/
