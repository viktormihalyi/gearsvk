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

USING_PTR_2 (ShaderModule);
class GEARSVK_API ShaderModule : public Noncopyable {
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
        GLSLFilePath,
        SPVFilePath,
        GLSLString,
    };

    struct Reflection {
        std::vector<SR::UBO>     ubos;
        std::vector<SR::Sampler> samplers;

        void Clear ()
        {
            ubos.clear ();
            samplers.clear ();
        }
    };

private:
    const VkDevice   device;
    VkShaderModule   handle;
    const ReadMode   readMode;
    const ShaderKind shaderKind;

    const std::string           sourceCode;
    const std::filesystem::path fileLocation;
    const std::vector<uint32_t> binary;

    // only available when compiled from GLSL
    Reflection reflection;

private:
    // private ctor, use factories instead
    ShaderModule (ShaderKind shaderKind, ReadMode mode, VkDevice device, VkShaderModule handle, const std::filesystem::path& fileLocation, const std::vector<uint32_t>& binary);

public:
    USING_PTR (ShaderModule);

    static ShaderModule::U CreateFromGLSLFile (VkDevice device, const std::filesystem::path& fileLocation);
    static ShaderModule::U CreateFromSPVFile (VkDevice device, const std::filesystem::path& fileLocation);
    static ShaderModule::U CreateFromGLSLString (VkDevice device, ShaderKind shaderKind, const std::string& shaderSource);

    virtual ~ShaderModule ();

    operator VkShaderModule () const { return handle; }

    void Reload ();

    const std::vector<uint32_t>& GetBinary () const { return binary; }

    const std::filesystem::path& GetLocation () const { return fileLocation; }

    ReadMode GetReadMode () const { return readMode; }

    VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo () const;

    const Reflection& GetReflection () const { return reflection; }
};

#endif