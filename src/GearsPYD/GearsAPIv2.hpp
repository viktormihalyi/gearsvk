#include "core/Sequence.h"

#include <cstdint>
#include <filesystem>
#include <functional>

#include "GearsAPI.hpp"
#include "Ptr.hpp"


class Presentable;
class VulkanEnvironment;
enum class ShaderKind : uint8_t;


void InitializeEnvironment ();

// exported for tests
GEARS_API
void DestroyEnvironment ();

void SetRenderGraphFromSequence (Sequence::P);

void StartRendering (const std::function<bool ()>&);

void TryCompile (ShaderKind shaderKind, const std::string& source);

intptr_t CreateSurface (intptr_t hwnd);

void DestroySurface (intptr_t handle);

void SetCurrentSurface (intptr_t handle);

// exported for tests
GEARS_API
void RenderFrame (uint32_t frameIndex);

std::string GetGLSLResourcesForRandoms ();


// for testing

GEARS_API
void SetOverriddenEnvironment (VulkanEnvironment&);

GEARS_API
void SetCurrentPresentable (Ptr<Presentable>&);

GEARS_API
void SetRenderGraphFromPyxFileSequence (const std::filesystem::path&);
