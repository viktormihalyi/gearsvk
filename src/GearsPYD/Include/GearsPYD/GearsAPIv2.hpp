#include <cstdint>
#include <functional>
#include <filesystem>
#include <memory>

#include "GearsPYD/GearsAPI.hpp"


namespace GVK {
enum class ShaderKind : uint8_t;
} // namespace GVK

namespace RG {
class Presentable;
class VulkanEnvironment;
} // namespace RG

class SequenceAdapter;
class Sequence;
class Stimulus;


namespace Gears {

void InitializeEnvironment ();

void DestroyEnvironment ();

std::string GetSpecs ();

void SetRenderGraphFromSequence (std::shared_ptr<Sequence>, const std::string& name);

void StartRendering ();

void TryCompile (GVK::ShaderKind shaderKind, const std::string& source);

intptr_t CreateSurface (intptr_t hwnd);

void DestroySurface (intptr_t handle);

void SetCurrentSurface (intptr_t handle);

void RenderFrame (uint32_t frameIndex);

void Wait ();

std::string GetGLSLResourcesForRandoms (const std::shared_ptr<Stimulus>& stimulus);

GEARS_API_TEST
std::unique_ptr<SequenceAdapter> GetSequenceAdapterFromPyx (RG::VulkanEnvironment&, const std::filesystem::path&);

GEARS_API_TEST
std::shared_ptr<Sequence> GetSequenceFromPyx (const std::filesystem::path&);

} // namespace Gears
