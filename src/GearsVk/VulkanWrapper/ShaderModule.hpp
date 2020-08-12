#ifndef SHADERMODULE_HPP
#define SHADERMODULE_HPP

#include "GearsVkAPI.hpp"

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "ShaderReflection.hpp"
#include "Utils.hpp"

#include <filesystem>

#include <vulkan/vulkan.h>

class GEARSVK_API ShaderCompileException : public std::runtime_error {
public:
    ShaderCompileException (const std::string& errorMessage)
        : std::runtime_error (errorMessage)
    {
    }
};

USING_PTR (ShaderModule);
class GEARSVK_API ShaderModule : public Noncopyable {
public:
    static constexpr uint32_t ShaderKindCount = 6;

    enum class ShaderKind {
        Vertex,
        Fragment,
        TessellationControl,
        TessellationEvaluation,
        Geometry,
        Compute,
    };

    enum class ReadMode {
        GLSLFilePath,
        SPVFilePath,
        GLSLString,
    };

    struct Reflection {
        std::vector<SR::UBOP>    ubos;
        std::vector<SR::Sampler> samplers;

        Reflection (const std::vector<uint32_t>& binary);
    };

private:
    const VkDevice   device;
    VkShaderModule   handle;
    const ReadMode   readMode;
    const ShaderKind shaderKind;

    const std::string           sourceCode;
    const std::filesystem::path fileLocation;
    const std::vector<uint32_t> binary;

    Reflection reflection;

private:
    // private ctor, use factories instead
    ShaderModule (ShaderKind shaderKind, ReadMode mode, VkDevice device, VkShaderModule handle, const std::filesystem::path& fileLocation, const std::vector<uint32_t>& binary);

public:
    USING_CREATE (ShaderModule);

    static ShaderModuleU CreateFromGLSLFile (VkDevice device, const std::filesystem::path& fileLocation);
    static ShaderModuleU CreateFromSPVFile (VkDevice device, ShaderKind shaderKind, const std::filesystem::path& fileLocation);
    static ShaderModuleU CreateFromGLSLString (VkDevice device, ShaderKind shaderKind, const std::string& shaderSource);

    virtual ~ShaderModule ();

    operator VkShaderModule () const { return handle; }

    void Reload ();

    const std::vector<uint32_t>& GetBinary () const { return binary; }

    const std::filesystem::path& GetLocation () const { return fileLocation; }

    ShaderKind GetShaderKind () const { return shaderKind; }

    ReadMode GetReadMode () const { return readMode; }

    VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo () const;

    const Reflection& GetReflection () const { return reflection; }
};

#endif