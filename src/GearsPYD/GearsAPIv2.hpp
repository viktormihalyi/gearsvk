#include "core/Sequence.h"

#include <cstdint>
#include <filesystem>
#include <functional>

#include "GearsAPI.hpp"
#include "Ptr.hpp"


class Presentable;
class VulkanEnvironment;
class SequenceAdapter;
enum class ShaderKind : uint8_t;


namespace Gears {

void InitializeEnvironment ();

void DestroyEnvironment ();

void SetRenderGraphFromSequence (Sequence::P);

void StartRendering (const std::function<bool ()>&);

void TryCompile (ShaderKind shaderKind, const std::string& source);

intptr_t CreateSurface (intptr_t hwnd);

void DestroySurface (intptr_t handle);

void SetCurrentSurface (intptr_t handle);

void RenderFrame (uint32_t frameIndex);

void Wait ();

std::string GetGLSLResourcesForRandoms ();

void SetCurrentPresentable (Ptr<Presentable>&);

GEARS_API_TEST
U<SequenceAdapter> GetSequenceAdapterFromPyx (VulkanEnvironment&, const std::filesystem::path&);

} // namespace Gears
