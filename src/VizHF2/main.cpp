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
#include "Utils.hpp"
#include "VulkanEnvironment.hpp"

// from VulkanWrapper
#include "VulkanWrapper.hpp"

#include "DeviceExtra.hpp"
#include "ShaderReflection.hpp"
#include "UniformReflection.hpp"

#include "CameraControl.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
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

class QuadricMat4 : public glm::mat4 {
public:
    QuadricMat4 ()
    {
    }

    QuadricMat4 (const glm::mat4& m)
    {
        glm::mat4::operator= (m);
    }

    QuadricMat4 Scale (const glm::vec3& v) const
    {
        return QuadricTransform (*this, glm::scale (UnitMatrix, v));
    }

    QuadricMat4 Rotate (float angle, const glm::vec3& v)
    {
        return QuadricTransform (*this, glm::rotate (UnitMatrix, angle, v));
    }

    QuadricMat4 Translate (const glm::vec3& v) const
    {
        return QuadricTransform (*this, glm::translate (UnitMatrix, v));
    }

private:
    static glm::mat4 QuadricTransform (const glm::mat4& surface, const glm::mat4& trafo)
    {
        const glm::mat4 tinv = glm::inverse (trafo);
        return glm::transpose (tinv) * surface * tinv;
    }
};

struct Quadric {
    QuadricMat4 surface;
    QuadricMat4 clipper;
    glm::vec4   kd;
    glm::vec4   reflectance;
    glm::vec4   ks;
    glm::vec4   transmittace;
};


int main (int, char**)
{
    WindowU window = GLFWWindow::Create ();

    VulkanEnvironmentU testenv = VulkanEnvironment::Create ();

    Device&      device        = *testenv->device;
    CommandPool& commandPool   = *testenv->commandPool;
    Queue&       graphicsQueue = *testenv->graphicsQueue;
    DeviceExtra& deviceExtra   = *testenv->deviceExtra;

    PresentableP presentable = testenv->CreatePresentable (*window);
    Swapchain&   swapchain   = presentable->GetSwapchain ();

    Camera        c (glm::vec3 (-0.553508, -4.64034, 8.00851), glm::vec3 (0.621111, 0.629845, -0.466387), window->GetAspectRatio ());
    CameraControl cameraControl (c, window->events);
    c.SetSpeed (3.f);

    RG::GraphSettings s (deviceExtra, swapchain.GetImageCount ());
    RG::RenderGraph   graph;


    // ========================= GRAPH OPERATIONS =========================

    ShaderPipelineP sp = ShaderPipeline::CreateShared (device);
    sp->SetShadersFromSourceFiles ({
        ShadersFolder / "quadric.vert",
        ShadersFolder / "quadric.frag",
    });

    RG::RenderOperationP brainRenderOp = graph.CreateOperation<RG::RenderOperation> (FullscreenQuad::CreateShared (deviceExtra), sp);

    // ========================= GRAPH RESOURCES =========================

    RG::SwapchainImageResourceP presented = graph.CreateResource<RG::SwapchainImageResource> (*presentable);

    RG::UniformReflection refl (graph);

    // ========================= GRAPH CONNECTIONS =========================

    graph.CreateOutputConnection (*brainRenderOp, 0, *presented);


    // ========================= GRAPH COMPILE =========================

    graph.Compile (s);

    // ========================= RENDERING =========================

    RG::SynchronizedSwapchainGraphRenderer renderer (s, swapchain);

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
            vkQueueWaitIdle (graph.GetGraphSettings ().GetDevice ().GetGraphicsQueue ());
            sp->Reload ();
            renderer.Recreate (graph);
        }
    };

    refl[brainRenderOp][ShaderKind::Vertex]["Camera"]["VP"]   = glm::mat4 (1.f);
    refl[brainRenderOp][ShaderKind::Fragment]["Camera"]["VP"] = glm::mat4 (1.f);

    enum class QuadricSurfaceType : uint32_t {
        Diffuse = 0,
        Tukor   = 1,
        Toro    = 2,
    };

    const QuadricMat4 zplane (glm::mat4 (
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f));

    const QuadricMat4 unitSphere (glm::mat4 (
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f));

    const QuadricMat4 cylinder (glm::mat4 (
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f));

    const QuadricMat4 hyperboloid (glm::mat4 (
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f));

    const QuadricMat4 paraboloid (glm::mat4 (
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.5f,
        0.0f, 0.0f, 0.5f, 0.0f));

    const QuadricMat4 all (glm::mat4 (
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f));

    const QuadricMat4 outside (glm::mat4 (
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f));

    struct Light {
        glm::vec4 position;
        glm::vec4 powerDensity;
    };

    std::vector<Quadric> quadrics;
    {
        quadrics.emplace_back ();
#define QLAST quadrics.size () - 1
        quadrics[QLAST].surface = zplane.Translate (glm::vec3 (0, 0, -3));
        quadrics[QLAST].clipper = all;
        quadrics[QLAST].kd      = glm::vec4 (0.1, 0.2, 0.2, 0);

        quadrics.emplace_back ();
        quadrics[QLAST].surface = unitSphere.Scale (glm::vec3 (1.2, 1.2, 0.5)).Translate (glm::vec3 (4, -3, 3));
        quadrics[QLAST].clipper = all;
        quadrics[QLAST].kd      = glm::vec4 (1, 0.1, 0.1, 0);
        quadrics[QLAST].ks      = glm::vec4 (0.5, 0.5, 0, 0);

        quadrics.emplace_back ();
        quadrics[QLAST].surface = paraboloid.Translate (glm::vec3 (4, 0, 3));
        quadrics[QLAST].clipper = unitSphere.Translate (glm::vec3 (4, 0, 3));
        quadrics[QLAST].kd      = glm::vec4 (0.1, 1, 0.1, 0);

        // idealias tukor
        quadrics.emplace_back ();
        quadrics[QLAST].surface     = unitSphere.Scale (glm::vec3 (8, 1, 5)).Translate (glm::vec3 (4, 14, 6));
        quadrics[QLAST].clipper     = all;
        quadrics[QLAST].reflectance = glm::vec4 (1, 1, 1, 0);

        // uveg
        quadrics.emplace_back ();
        quadrics[QLAST].surface      = unitSphere.Translate (glm::vec3 (4, 0, 6));
        quadrics[QLAST].clipper      = all;
        quadrics[QLAST].reflectance  = glm::vec4 (0.75, 0.75, 0.75, 0);
        quadrics[QLAST].transmittace = glm::vec4 (0.75, 0.75, 0.75, 0);

        // idealis toro
        quadrics.emplace_back ();
        quadrics[QLAST].surface      = unitSphere.Translate (glm::vec3 (4, -3, 6));
        quadrics[QLAST].clipper      = all;
        quadrics[QLAST].transmittace = glm::vec4 (1, 1, 1, 0);
    }

    std::vector<Light> lights;
    {
        lights.emplace_back ();
        lights[0].position     = glm::normalize (glm::vec4 (0.6, -0.6, 1, 0));
        lights[0].powerDensity = glm::vec4 (0.1, 0.1, 0.1, 0);

        lights.emplace_back ();
        lights[1].position     = glm::vec4 (10, 0, 10, 1);
        lights[1].powerDensity = glm::vec4 (0.95, 0.87, 0.8, 0) * 300.f;

        lights.emplace_back ();
        lights[2].position     = glm::vec4 (0, 0, 10, 1);
        lights[2].powerDensity = glm::vec4 (0.1, 0.87, 0.34, 0) * 300.f;

        lights.emplace_back ();
        lights[3].position     = glm::vec4 (5, 5, 4, 1);
        lights[3].powerDensity = glm::vec4 (187 / 255.f, 143 / 255.f, 206 / 255.f, 0) * 100.f;
    }

    refl[brainRenderOp][ShaderKind::Fragment]["Quadrics"] = quadrics;
    refl[brainRenderOp][ShaderKind::Fragment]["Lights"]   = lights;


    renderer.preSubmitEvent += [&] (RG::RenderGraph&, uint32_t frameIndex, uint64_t deltaNs) {
        TimePoint delta (deltaNs);

        const float dt = delta.AsSeconds ();

        cameraControl.UpdatePosition (dt);

        {
            refl[brainRenderOp][ShaderKind::Vertex]["Camera"]["viewMatrix"]   = c.GetViewMatrix ();
            refl[brainRenderOp][ShaderKind::Vertex]["Camera"]["rayDirMatrix"] = c.GetRayDirMatrix ();
            refl[brainRenderOp][ShaderKind::Vertex]["Camera"]["camPosition"]  = c.GetPosition ();
            refl[brainRenderOp][ShaderKind::Vertex]["Camera"]["viewDir"]      = c.GetViewDirection ();

            refl[brainRenderOp][ShaderKind::Fragment]["Camera"]["viewMatrix"]   = c.GetViewMatrix ();
            refl[brainRenderOp][ShaderKind::Fragment]["Camera"]["rayDirMatrix"] = c.GetRayDirMatrix ();
            refl[brainRenderOp][ShaderKind::Fragment]["Camera"]["position"]     = c.GetPosition ();
            refl[brainRenderOp][ShaderKind::Fragment]["Camera"]["viewDir"]      = c.GetViewDirection ();
        }

        refl.Flush (frameIndex);
    };

    window->DoEventLoop (renderer.GetConditionalDrawCallback ([&] () -> RG::RenderGraph& { return graph; }, [&] { return quit; }));
}
