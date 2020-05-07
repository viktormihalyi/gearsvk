#include "core/Sequence.h"

#include <functional>

#include "ShaderModule.hpp"

void InitializeEnvironment ();

void DestroyEnvironment ();

// not exported, only called from old api
void SetRenderGraphFromSequence (Sequence::P);

void StartRendering (const std::function<bool ()>&);

void TryCompile (ShaderModule::ShaderKind shaderKind, const std::string& source);
