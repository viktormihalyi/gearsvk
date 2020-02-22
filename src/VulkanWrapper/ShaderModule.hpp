#ifndef VW_SHADERMODULE_HPP
#define VW_SHADERMODULE_HPP

#include "DeviceHolder.hpp"
#include "Ptr.hpp"

#include <filesystem>

#include <vulkan/vulkan.h>


namespace VW {

DEFINE_PTR (ShaderModule);

class ShaderModule final : public DeviceHolder {
private:
    VkShaderModule handle;

public:
    ShaderModule (VkDevice device, const std::filesystem::path& binaryFilePath);
    ~ShaderModule () override;
};

} // namespace VW


#endif