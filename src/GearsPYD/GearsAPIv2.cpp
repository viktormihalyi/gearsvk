#include "GearsAPIv2.hpp"

// from GearsVk
#include "Instance.hpp"
#include "ShaderModule.hpp"
#include "Surface.hpp"
#include "VulkanEnvironment.hpp"
#include "Window.hpp"

// from Gears
#include "SequenceAdapter.hpp"
#include "core/Sequence.h"

// from pybind11
#include <pybind11/embed.h>


namespace Gears {


static SequenceAdapterU              currentSeq;
static std::vector<Ptr<Presentable>> createdSurfaces;

static VulkanEnvironment& GetVkEnvironment ();
static void               DestroyVkEnvironment ();


void InitializeEnvironment ()
{
    GetVkEnvironment ();
}


void DestroyEnvironment ()
{
    createdSurfaces.clear ();
    currentSeq.reset ();
    DestroyVkEnvironment ();
}


void SetRenderGraphFromSequence (Sequence::P seq)
{
    currentSeq = SequenceAdapter::Create (GetVkEnvironment (), seq);
}


void StartRendering (const std::function<bool ()>& doRender)
{
    currentSeq->RenderFullOnExternalWindow ();
}


void TryCompile (ShaderKind shaderKind, const std::string& source)
{
    try {
        ShaderModule::CreateFromGLSLString (*GetVkEnvironment ().device, shaderKind, source);
        std::cout << "compile succeeded" << std::endl;
    } catch (const ShaderCompileException&) {
        std::cout << "compile failed, source code: " << std::endl;
        std::cout << source << std::endl;
    }
}


intptr_t CreateSurface (intptr_t hwnd)
{
#ifdef WIN32
    Ptr<Presentable> presentable = Presentable::Create (GetVkEnvironment (), Surface::Create (Surface::PlatformSpecific, *GetVkEnvironment ().instance, reinterpret_cast<void*> (hwnd)));
    createdSurfaces.push_back (presentable);
    return reinterpret_cast<intptr_t> (presentable.get ());
#else
    GVK_BREAK ("Creating native VkSurfaceKHR on this platform is not supported.");
    return 0;
#endif
}


static Ptr<Presentable> GetSurfaceFromHandle (intptr_t surfaceHandle)
{
    for (const Ptr<Presentable>& createdSurface : createdSurfaces) {
        if (reinterpret_cast<intptr_t> (createdSurface.get ()) == surfaceHandle) {
            return createdSurface;
        }
    }
    return nullptr;
}


void SetCurrentSurface (intptr_t surfaceHandle)
{
    Ptr<Presentable> presentable = GetSurfaceFromHandle (surfaceHandle);
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
    createdSurfaces.erase (std::remove_if (createdSurfaces.begin (), createdSurfaces.end (), [&] (const Ptr<Presentable>& p) {
                               return reinterpret_cast<intptr_t> (p.get ()) == surfaceHandle;
                           }),
                           createdSurfaces.end ());
}


std::string GetGLSLResourcesForRandoms ()
{
    return R"(
#ifndef GEARS_RANDOMS_RESOURCES
#define GEARS_RANDOMS_RESOURCES
#define RANDOMS_ARRAY_SIZE 5
    layout (binding = 201) uniform usampler2D randoms[RANDOMS_ARRAY_SIZE];
    layout (binding = 202) uniform ubo_cellSize { vec2 cellSize; };
    layout (binding = 203) uniform ubo_randomGridSize { ivec2 randomGridSize; };
    layout (binding = 204) uniform ubo_randomsIndex { uint randomsIndex; };
#endif
    )";
}


SequenceAdapterU GetSequenceAdapterFromPyx (VulkanEnvironment& environment, const std::filesystem::path& filePath)
{
    if (GVK_ERROR (!std::filesystem::exists (filePath))) {
        return nullptr;
    }

    // working directory will be the same as PROJECT_ROOT

    pybind11::scoped_interpreter guard;
    try {
        pybind11::module sys = pybind11::module::import ("sys");

        sys.attr ("path").attr ("insert") (0, (PROJECT_ROOT / "src" / "UserInterface").u8string ());

        pybind11::module::import ("AppData").attr ("initConfigParams") ();

        pybind11::module sequenceLoader = pybind11::module::import ("SequenceLoaderCore");

        sequenceLoader.attr ("loadParents") (filePath.parent_path ().u8string (), (PROJECT_ROOT / "src" / "UserInterface" / "Project").u8string ());

        pybind11::module machinery        = pybind11::module::import ("importlib.machinery");
        pybind11::object sourceFileLoader = machinery.attr ("SourceFileLoader");

        pybind11::object sequenceCreator = sourceFileLoader ("my_module", filePath.u8string ()).attr ("load_module") ();

        pybind11::object sequence = sequenceCreator.attr ("create") (pybind11::none ());

        Sequence::P sequenceCpp = sequence.cast<Sequence::P> ();

        return SequenceAdapter::Create (environment, sequenceCpp);

    } catch (std::exception& e) {
        GVK_BREAK ("Failed to load sequence.");
        std::cout << e.what () << std::endl;
        return nullptr;
    }
}


void SetCurrentPresentable (Ptr<Presentable>& p)
{
    currentSeq->SetCurrentPresentable (p);
}


static VulkanEnvironmentU env_ = nullptr;


static VulkanEnvironment& GetVkEnvironment ()
{
    if (env_ == nullptr) {
        env_ = VulkanEnvironment::Create ();
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
