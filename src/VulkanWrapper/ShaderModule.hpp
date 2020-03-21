#ifndef SHADERMODULE_HPP
#define SHADERMODULE_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "ShaderReflection.hpp"
#include "Utils.hpp"

#include <filesystem>

#include <vulkan/vulkan.h>


class ShaderModule : public Noncopyable {
public:
    enum class ShaderKind {
        Vertex,
        Fragment,
        TessellationControl,
        TessellationEvaluation,
        Geometry,
        Compute,
    };

private:
    enum class ReadMode {
        Source,
        Binary,
        String,
    };

    const ReadMode readMode;

    const VkDevice              device;
    VkShaderModule              handle;
    const std::vector<uint32_t> binary;

    const ShaderKind            shaderKind;
    const std::filesystem::path fileLocation;

private:
    // private ctor, use factories
    ShaderModule (ShaderKind shaderKind, ReadMode mode, VkDevice device, VkShaderModule handle, const std::filesystem::path& fileLocation, const std::vector<uint32_t>& binary);

public:
    USING_PTR (ShaderModule);

    static ShaderModule::U CreateFromSource (VkDevice device, const std::filesystem::path& fileLocation);
    static ShaderModule::U CreateFromBinary (VkDevice device, const std::filesystem::path& fileLocation);
    static ShaderModule::U CreateFromString (VkDevice device, const std::string& shaderSource, ShaderKind shaderKind);

    virtual ~ShaderModule ();

    operator VkShaderModule () const { return handle; }

    void Reload ();

    const std::vector<uint32_t>& GetBinary () const { return binary; }

    VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo () const;
};

#endif