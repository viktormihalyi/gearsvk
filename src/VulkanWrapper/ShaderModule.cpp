#include "ShaderModule.hpp"
#include "Assert.hpp"

#include "Utils.hpp"

namespace VW {

ShaderModule::ShaderModule (VkDevice device, const std::filesystem::path& binaryFilePath)
    : DeviceHolder (device)
    , handle (VK_NULL_HANDLE)
{
    std::optional<std::vector<char>> shaderBytecode = Utils::ReadBinaryFile (binaryFilePath);

    if (ERROR (!shaderBytecode.has_value ())) {
        return;
    }

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize                 = shaderBytecode->size ();
    createInfo.pCode                    = reinterpret_cast<const uint32_t*> (shaderBytecode->data ());

    VkResult result = vkCreateShaderModule (device, &createInfo, nullptr, &handle);
    if (ERROR (result != VK_SUCCESS)) {
        return;
    }
}

ShaderModule::~ShaderModule ()
{
    if (handle != VK_NULL_HANDLE) {
        vkDestroyShaderModule (device, handle, nullptr);
    }
}

} // namespace VW
