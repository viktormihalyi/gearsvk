#include "Assert.hpp"
#include "Camera.hpp"
#include "FullscreenQuad.hpp"
#include "GLFWWindow.hpp"
#include "GraphRenderer.hpp"
#include "GraphSettings.hpp"
#include "Logger.hpp"
#include "MultithreadedFunction.hpp"
#include "Noncopyable.hpp"
#include "Operation.hpp"
#include "RenderGraph.hpp"
#include "Resource.hpp"
#include "SDLWindow.hpp"
#include "Time.hpp"
#include "Timer.hpp"
#include "UniformBlock.hpp"
#include "Utils.hpp"
#include "VulkanEnvironment.hpp"

// from VulkanWrapper
#include "VulkanWrapper.hpp"

#include "DeviceExtra.hpp"
#include "ShaderReflection.hpp"

#include "CameraControl.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <vector>

#include "glmlib.hpp"


const std::filesystem::path ShadersFolder = PROJECT_ROOT / "src" / "VizHF2" / "shaders";

static const glm::mat4 UnitMatrix = glm::mat4 (
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f);

struct Quadric {
    glm::mat4 surface;
    glm::mat4 clipper;
    glm::vec4 kd;
    glm::vec4 kr;
    glm::vec4 ks;
    glm::vec4 d2;

    static glm::mat4 QuadricTransform (const glm::mat4& surface, const glm::mat4& trafo)
    {
        glm::mat4 tinv = glm::inverse (trafo);
        return glm::transpose (tinv) * surface * tinv;
    }

    Quadric& Scale (const glm::vec3& v)
    {
        surface = QuadricTransform (surface, glm::scale (UnitMatrix, v));
        return *this;
    }

    Quadric& Rotate (float angle, const glm::vec3& v)
    {
        surface = QuadricTransform (surface, glm::rotate (UnitMatrix, angle, v));
        return *this;
    }

    Quadric& Translate (const glm::vec3& v)
    {
        surface = QuadricTransform (surface, glm::translate (UnitMatrix, v));
        return *this;
    }
};


int main (int, char**)
{
    Window::U window = GLFWWindow::Create ();

    VulkanEnvironment::U testenv = VulkanEnvironment::Create (*window);

    Device&      device        = *testenv->device;
    CommandPool& commandPool   = *testenv->commandPool;
    Queue&       graphicsQueue = *testenv->graphicsQueue;
    Swapchain&   swapchain     = *testenv->swapchain;
    DeviceExtra& deviceExtra   = *testenv->deviceExtra;

    Camera        c (glm::vec3 (-1, 0, 0.5f), glm::vec3 (1, 0.0f, 0), window->GetAspectRatio ());
    CameraControl cameraControl (c, window->events);
    c.SetSpeed (3.f);

    const RG::GraphSettings s (device, graphicsQueue, commandPool, swapchain);
    RG::RenderGraph         graph (device, commandPool);


    // ========================= GRAPH OPERATIONS =========================

    ShaderPipeline::P sp = ShaderPipeline::CreateShared (device);
    sp->SetShadersFromSourceFiles ({
        ShadersFolder / "quadratic.vert",
        ShadersFolder / "quadratic.frag",
    });

    RG::RenderOperation& brainRenderOp = graph.CreateOperation<RG::RenderOperation> (FullscreenQuad::CreateShared (deviceExtra), sp);


    // ========================= GRAPH RESOURCES =========================

    RG::SwapchainImageResource&    presented = graph.CreateResource<RG::SwapchainImageResource> (swapchain);
    RG::UniformReflectionResource& refl      = graph.CreateResource<RG::UniformReflectionResource> (sp, RG::UniformReflectionResource::Strategy::UniformBlocksOnly);


    // ========================= GRAPH CONNECTIONS =========================

    for (uint32_t i = 0; i < refl.uboRes.size (); ++i) {
        graph.CreateInputConnection<RG::UniformInputBinding> (brainRenderOp, refl.bindings[i], *refl.uboRes[i]);
    }
    graph.CreateOutputConnection (brainRenderOp, 0, presented);


    // ========================= GRAPH COMPILE =========================

    graph.Compile (s);

    // ========================= RENDERING =========================

    RG::SynchronizedSwapchainGraphRenderer renderer (graph, swapchain);

    bool quit = false;

    window->events.keyPressed += [&] (uint32_t key) {
        constexpr uint8_t ESC_CODE = 27;
        if (key == ESC_CODE) {
            // window->ToggleFullscreen ();
            quit = true;
        }
        if (key == 'R') {
            std::cout << "waiting for device... " << std::endl;
            vkDeviceWaitIdle (graph.GetGraphSettings ().GetDevice ());
            vkQueueWaitIdle (graph.GetGraphSettings ().queue);
            sp->Reload ();
            renderer.Recreate ();
        }
    };

    refl.vert["Camera"]["VP"] = glm::mat4 (1.f);
    refl.frag["Camera"]["VP"] = glm::mat4 (1.f);

    enum class QuadricSurfaceType : uint32_t {
        Diffuse = 0,
        Tukor   = 1,
        Toro    = 2,
    };

    constexpr glm::mat4 zplane = glm::mat4 (
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f);

    constexpr glm::mat4 unitSphere = glm::mat4 (
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f);

    constexpr glm::mat4 halfUnitSphere = glm::mat4 (
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -0.5f);

    constexpr glm::mat4 all = glm::mat4 (
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f);

    struct Light {
        glm::vec4 position;
        glm::vec4 powerDensity;
    };

    std::vector<Quadric> quadrics;
    {
        quadrics.emplace_back ();
#define LAST quadrics.size () - 1
        quadrics[LAST].surface = zplane;
        quadrics[LAST].clipper = all;
        quadrics[LAST].kd      = glm::vec4 (0.2, 0.2, 1, 0);
        quadrics[LAST].kr      = glm::vec4 (0, 0, 0, 0);
        quadrics[LAST].ks      = glm::vec4 (0, 0, 0, 0);
        quadrics[LAST].Translate (glm::vec3 (0, 0, -3));

        quadrics.emplace_back ();
        quadrics[LAST].surface = unitSphere;
        quadrics[LAST].clipper = all;
        quadrics[LAST].kd      = glm::vec4 (1, 0.1, 0.1, 0);
        quadrics[LAST].kr      = glm::vec4 (0, 0, 0, 0);
        quadrics[LAST].ks      = glm::vec4 (1, 1, 1, 0);
        quadrics[LAST].Translate (glm::vec3 (4, -3, 3));

        quadrics.emplace_back ();
        quadrics[LAST].surface = unitSphere;
        quadrics[LAST].clipper = all;
        quadrics[LAST].kd      = glm::vec4 (0.1, 1, 0.1, 0);
        quadrics[LAST].kr      = glm::vec4 (0, 0, 0, 0);
        quadrics[LAST].ks      = glm::vec4 (0, 0, 0, 0);
        quadrics[LAST].Translate (glm::vec3 (4, 0, 3));

        //tukor
        quadrics.emplace_back ();
        quadrics[LAST].surface = unitSphere;
        quadrics[LAST].clipper = all;
        quadrics[LAST].kd      = glm::vec4 (0, 0, 0, 0);
        quadrics[LAST].kr      = glm::vec4 (1, 1, 1, 0);
        quadrics[LAST].ks      = glm::vec4 (0, 0, 0, 0);
        quadrics[LAST].Translate (glm::vec3 (4, 3, 3));

        //uveg
        quadrics.emplace_back ();
        quadrics[LAST].surface = unitSphere;
        quadrics[LAST].clipper = all;
        quadrics[LAST].kd = glm::vec4 (0, 0, 0, 0);
        quadrics[LAST].kr = glm::vec4 (0.5, 0.5, 0.5, 0);
        quadrics[LAST].ks = glm::vec4 (0, 0, 0, 0);
        quadrics[LAST].Translate (glm::vec3 (7, 0, 3));
    }

    std::vector<Light> lights;
    {
        lights.emplace_back ();
        lights[0].position     = glm::normalize (glm::vec4 (0, 1, 1, 0));
        lights[0].powerDensity = glm::vec4 (0, 0, 0, 0);

        lights.emplace_back ();
        lights[1].position     = glm::normalize (glm::vec4 (1, 0, 1, 0));
        lights[1].powerDensity = glm::vec4 (1, 1, 1, 0);
    }

    refl.frag["Quadrics"] = quadrics;
    refl.frag["Lights"]   = lights;

    renderer.preSubmitEvent += [&] (uint32_t frameIndex, uint64_t deltaNs) {
        TimePoint delta (deltaNs);

        const float dt = delta.AsSeconds ();

        cameraControl.UpdatePosition (dt);

        {
            refl.vert["Camera"]["viewMatrix"]   = c.GetViewMatrix ();
            refl.vert["Camera"]["rayDirMatrix"] = c.GetRayDirMatrix ();
            refl.vert["Camera"]["camPosition"]  = c.GetPosition ();
            refl.vert["Camera"]["viewDir"]      = c.GetViewDirection ();

            refl.frag["Camera"]["viewMatrix"]   = c.GetViewMatrix ();
            refl.frag["Camera"]["rayDirMatrix"] = c.GetRayDirMatrix ();
            refl.frag["Camera"]["position"]     = c.GetPosition ();
            refl.frag["Camera"]["viewDir"]      = c.GetViewDirection ();
        }

        refl.Update (frameIndex);
    };

    window->DoEventLoop (renderer.GetConditionalDrawCallback ([&] { return quit; }));
}
