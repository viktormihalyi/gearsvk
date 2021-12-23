#ifndef RENDERGRAPH_SHADERREFLECTIONTOVERTEXATTRIBUE_HPP
#define RENDERGRAPH_SHADERREFLECTIONTOVERTEXATTRIBUE_HPP

#include "RenderGraphExport.hpp"

#include "VulkanWrapper/ShaderModule.hpp"

#include <vulkan/vulkan.h>

#include <vector>
#include <functional>
#include <string>


namespace RG {
namespace FromShaderReflection {

RENDERGRAPH_DLL_EXPORT
std::vector<VkVertexInputAttributeDescription> GetVertexAttributes (const GVK::ShaderModule::Reflection& reflection, const std::function<bool (const std::string&)>& instanceNameProvider);

RENDERGRAPH_DLL_EXPORT
std::vector<VkVertexInputBindingDescription> GetVertexBindings (const GVK::ShaderModule::Reflection& reflection, const std::function<bool (const std::string&)>& instanceNameProvider);

} // namespace FromShaderReflection
} // namespace RG

#endif
