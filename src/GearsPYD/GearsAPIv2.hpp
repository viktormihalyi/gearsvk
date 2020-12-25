#include "core/Sequence.h"

#include <cstdint>
#include <filesystem>
#include <functional>

#include "GearsAPI.hpp"

#include "ShaderModule.hpp"

void InitializeEnvironment ();

void DestroyEnvironment ();

// not exported, only called from old api
void SetRenderGraphFromSequence (Sequence::P);
void GEARS_API SetRenderGraphFromPyxFileSequence (const std::filesystem::path&);

void StartRendering (const std::function<bool ()>&);

void TryCompile (ShaderKind shaderKind, const std::string& source);

intptr_t CreateSurface (intptr_t hwnd);

void DestroySurface (intptr_t handle);

void SetCurrentSurface (intptr_t handle);

void RenderFrame (uint32_t frameIndex);

std::string GetGLSLResourcesForRandoms ();
