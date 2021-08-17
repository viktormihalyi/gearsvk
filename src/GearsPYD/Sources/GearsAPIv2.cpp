#include "GearsAPIv2.hpp"

// from RenderGraph
#include "RenderGraph/GraphRenderer.hpp"
#include "VulkanWrapper/Instance.hpp"
#include "VulkanWrapper/ShaderModule.hpp"
#include "VulkanWrapper/Surface.hpp"
#include "RenderGraph/VulkanEnvironment.hpp"
#include "RenderGraph/Window/Window.hpp"
#include "RenderGraph/Window/GLFWWindow.hpp"

// from Gears
#include "Sequence/SequenceAdapter.hpp"
#include "Sequence/StimulusAdapter.hpp"
#include "PySequence/core/PySequence.h"

// from pybind11
#include <pybind11/embed.h>

// from spdlog
#include "spdlog/spdlog.h"

namespace Gears {


static std::unique_ptr<SequenceAdapter>               currentSeq;
static std::vector<std::shared_ptr<GVK::Presentable>> createdSurfaces;

static GVK::VulkanEnvironment& GetVkEnvironment ();
static void                    DestroyVkEnvironment ();


void InitializeEnvironment ()
{
    GetVkEnvironment ();
}


void DestroyEnvironment ()
{
    GetVkEnvironment ().Wait ();
    createdSurfaces.clear ();
    currentSeq.reset ();
    DestroyVkEnvironment ();
}


void SetRenderGraphFromSequence (std::shared_ptr<Sequence> seq, const std::string& name)
{
    currentSeq = std::make_unique<SequenceAdapter> (GetVkEnvironment (), seq, name);
}


void StartRendering ()
{
    try {
        currentSeq->RenderFullOnExternalWindow ();
    } catch (std::exception& ex) {
        spdlog::error ("StartRendering failed: {}", ex.what ());
        GVK_BREAK ("rendering failed");
    } catch (...) {
        GVK_BREAK ("rendering failed!!");
    }
}


void TryCompile (GVK::ShaderKind shaderKind, const std::string& source)
{
    try {
        GVK::ShaderModule::CreateFromGLSLString (*GetVkEnvironment ().device, shaderKind, source);
    } catch (const GVK::ShaderCompileException&) {
        spdlog::info ("compile failed, source code:\n{}", source);
    }
}


intptr_t CreateSurface (intptr_t hwnd)
{
#ifdef WIN32
    std::shared_ptr<GVK::Presentable> presentable =
        std::make_unique<GVK::Presentable> (GetVkEnvironment (),
                                            std::make_unique<GVK::Surface> (GVK::Surface::PlatformSpecific, *GetVkEnvironment ().instance, reinterpret_cast<void*> (hwnd)),
                                            std::make_unique<GVK::DefaultSwapchainSettings> ());

    createdSurfaces.push_back (presentable);
    return reinterpret_cast<intptr_t> (presentable.get ());
#else
    GVK_BREAK ("Creating native VkSurfaceKHR on this platform is not supported.");
    return 0;
#endif
}


static std::shared_ptr<GVK::Presentable> GetSurfaceFromHandle (intptr_t surfaceHandle)
{
    for (const std::shared_ptr<GVK::Presentable>& createdSurface : createdSurfaces) {
        if (reinterpret_cast<intptr_t> (createdSurface.get ()) == surfaceHandle) {
            return createdSurface;
        }
    }
    return nullptr;
}


void SetCurrentSurface (intptr_t surfaceHandle)
{
    std::shared_ptr<GVK::Presentable> presentable = GetSurfaceFromHandle (surfaceHandle);
    if (GVK_ERROR (presentable == nullptr)) {
        return;
    }

    if (GVK_ERROR (currentSeq == nullptr)) {
        return;
    }

    currentSeq->SetCurrentPresentable (presentable);
}


void RenderFrame (uint32_t frameIndex)
{
    currentSeq->RenderFrameIndex (frameIndex);
}


void Wait ()
{
    currentSeq->Wait ();
}


void DestroySurface (intptr_t surfaceHandle)
{
    const auto FindPresentable = [&] (const std::shared_ptr<GVK::Presentable>& p) {
        return reinterpret_cast<intptr_t> (p.get ()) == surfaceHandle;
    };

    createdSurfaces.erase (std::remove_if (createdSurfaces.begin (), createdSurfaces.end (), FindPresentable),
                           createdSurfaces.end ());
}


std::string GetGLSLResourcesForRandoms ()
{
    return R"(
#ifndef GEARS_RANDOMS_RESOURCES
#define GEARS_RANDOMS_RESOURCES
    layout (binding = 201) uniform usampler2D randoms;
    layout (binding = 202) uniform randomUniformBlock {
        vec2    cellSize;
        ivec2   randomGridSize;
        uint    randomsIndex;
    };
#endif

    )";
}


std::shared_ptr<Sequence> GetSequenceFromPyx (const std::filesystem::path& filePath)
{
    if (GVK_ERROR (!std::filesystem::exists (filePath))) {
        return nullptr;
    }

    pybind11::scoped_interpreter guard;
    try {
        pybind11::module sys = pybind11::module::import ("sys");

        sys.attr ("path").attr ("insert") (0, (std::filesystem::current_path ()).string ());

        pybind11::module::import ("AppData").attr ("initConfigParams") ();

        pybind11::module sequenceLoader = pybind11::module::import ("SequenceLoaderCore");

        sequenceLoader.attr ("loadParents") (filePath.parent_path ().string (), (std::filesystem::current_path () / "Project").string ());

        pybind11::module machinery        = pybind11::module::import ("importlib.machinery");
        pybind11::object sourceFileLoader = machinery.attr ("SourceFileLoader");

        pybind11::object sequenceCreator = sourceFileLoader ("my_module", filePath.string ()).attr ("load_module") ();

        pybind11::object sequence = sequenceCreator.attr ("create") (pybind11::none ());

        std::shared_ptr<PySequence> sequenceCpp = sequence.cast<std::shared_ptr<PySequence>> ();

        return sequenceCpp;

    } catch (std::exception& e) {
        GVK_BREAK ("Failed to load sequence.");
        spdlog::error ("Failed to load sequence: {}", e.what ());
        return nullptr;
    }
}


std::unique_ptr<SequenceAdapter> GetSequenceAdapterFromPyx (GVK::VulkanEnvironment& environment, const std::filesystem::path& filePath)
{
    std::shared_ptr<Sequence> sequence = GetSequenceFromPyx (filePath);
    return std::make_unique<SequenceAdapter> (environment, sequence, filePath.filename ().string ());
}


void SetCurrentPresentable (std::shared_ptr<GVK::Presentable>& p)
{
    currentSeq->SetCurrentPresentable (p);
}


static std::unique_ptr<GVK::VulkanEnvironment> env_ = nullptr;


static GVK::VulkanEnvironment& GetVkEnvironment ()
{
    if (env_ == nullptr) {
        env_ = std::make_unique<GVK::VulkanEnvironment> (GVK::defaultDebugCallback, GVK::GetGLFWInstanceExtensions (), std::vector<const char*> { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
    }

    return *env_;
}


static void DestroyVkEnvironment ()
{
    if (env_ != nullptr) {
        env_.reset ();
    }
}


} // namespace Gears
