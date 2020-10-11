#include "core/Sequence.h"

#include <functional>
#include <cstdint>

#include "ShaderModule.hpp"

void InitializeEnvironment ();

void DestroyEnvironment ();

// not exported, only called from old api
void SetRenderGraphFromSequence (Sequence::P);

void StartRendering (const std::function<bool ()>&);

void TryCompile (ShaderKind shaderKind, const std::string& source);

void RenderFrame (uint32_t frameIndex);

void CreateSurface (intptr_t hwnd);
