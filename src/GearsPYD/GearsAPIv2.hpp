#include "core/Sequence.h"

#include <cstdint>
#include <functional>

#include "ShaderModule.hpp"

void InitializeEnvironment ();

void DestroyEnvironment ();

// not exported, only called from old api
void SetRenderGraphFromSequence (Sequence::P);

void StartRendering (const std::function<bool ()>&);

void TryCompile (ShaderKind shaderKind, const std::string& source);

intptr_t CreateSurface (intptr_t hwnd);

void DestroySurface (intptr_t handle);

void SetCurrentSurface (intptr_t handle);

void RenderFrame (uint32_t frameIndex);

void RequestPaint (intptr_t handle);

std::string GetGLSLResourcesForRandoms ();
