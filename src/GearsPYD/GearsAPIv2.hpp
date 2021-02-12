#include <cstdint>
#include <filesystem>
#include <functional>

#include "GearsAPI.hpp"
#include "Ptr.hpp"

namespace GVK {
class Presentable;
class VulkanEnvironment;
enum class ShaderKind : uint8_t;
} // namespace GVK
class SequenceAdapter;
class Sequence;


namespace Gears {

void InitializeEnvironment ();

void DestroyEnvironment ();

void SetRenderGraphFromSequence (Ptr<Sequence>);

void StartRendering ();

void TryCompile (GVK::ShaderKind shaderKind, const std::string& source);

intptr_t CreateSurface (intptr_t hwnd);

void DestroySurface (intptr_t handle);

void SetCurrentSurface (intptr_t handle);

void RenderFrame (uint32_t frameIndex);

void Wait ();

std::string GetGLSLResourcesForRandoms ();

void SetCurrentPresentable (Ptr<GVK::Presentable>&);

GEARS_API_TEST
U<SequenceAdapter> GetSequenceAdapterFromPyx (GVK::VulkanEnvironment&, const std::filesystem::path&);

} // namespace Gears
