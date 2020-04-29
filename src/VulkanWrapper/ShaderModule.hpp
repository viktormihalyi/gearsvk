#ifndef SHADERMODULE_HPP
#define SHADERMODULE_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "ShaderReflection.hpp"
#include "Utils.hpp"

#include <filesystem>

#include <vulkan/vulkan.h>

class ShaderCompileException : public std::runtime_error {
public:
    ShaderCompileException (const std::string& errorMessage)
        : std::runtime_error (errorMessage)
    {
    }
};

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

    enum class ReadMode {
        Source,
        Binary,
        String,
    };

private:
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
    static ShaderModule::U CreateFromString (VkDevice device, ShaderKind shaderKind, const std::string& shaderSource);

    virtual ~ShaderModule ();

    operator VkShaderModule () const { return handle; }

    void Reload ();

    const std::vector<uint32_t>& GetBinary () const { return binary; }

    const std::filesystem::path& GetLocation () const { return fileLocation; }

    ReadMode GetReadMode () const { return readMode; }

    VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo () const;
};

#endif