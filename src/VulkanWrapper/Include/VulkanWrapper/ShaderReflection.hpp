#ifndef SHADERREFLECTION_HPP
#define SHADERREFLECTION_HPP

#include "VulkanWrapper/VulkanWrapperAPI.hpp"
#include "Utils/Noncopyable.hpp"
#include <memory>

#include <cstdint>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>


// NOTE: SSBOs are not supported

namespace GVK {

namespace SR {


// clang-format off

enum class FieldType : uint16_t {
    // Scalars
    Bool,
    Int,
    Uint,
    Float,
    Double,
    i64,
    u64,

    // vectors
    Bvec2, Ivec2, Uvec2, Vec2, Dvec2, i64_vec2, u64_vec2,
    Bvec3, Ivec3, Uvec3, Vec3, Dvec3, i64_vec3, u64_vec3,
    Bvec4, Ivec4, Uvec4, Vec4, Dvec4, i64_vec4, u64_vec4,

    // float matrices
    Mat2x2, Mat2x3, Mat2x4,
    Mat3x2, Mat3x3, Mat3x4,
    Mat4x2, Mat4x3, Mat4x4,

    // double matrices
    Dmat2x2, Dmat2x3, Dmat2x4,
    Dmat3x2, Dmat3x3, Dmat3x4,
    Dmat4x2, Dmat4x3, Dmat4x4,

    // ...
    Struct,
    Unknown
};

// clang-format on

VULKANWRAPPER_API
std::string FieldTypeToString (FieldType fieldType);


class Field;

class VULKANWRAPPER_API FieldContainer {
public:
    virtual ~FieldContainer () = default;

    virtual const std::vector<std::unique_ptr<Field>>& GetFields () const = 0;
};


class VULKANWRAPPER_API Field final : public FieldContainer, public Noncopyable {
public:
    std::string name;
    FieldType   type;

    // with respect to parent struct
    uint32_t offset;

    // 0 for structs
    // single element size for arrays
    // otherwise the number of bytes a variable takes up (eg. 4 for floats)
    uint32_t size;

    uint32_t arraySize;   // 0 for non-arrays
    uint32_t arrayStride; // 0 for non-arrays

    std::vector<std::unique_ptr<Field>> structFields; // when type == FieldType::Struct

    Field ();

    bool IsArray () const;

    bool IsStruct () const;

    uint32_t GetSize () const;

    virtual const std::vector<std::unique_ptr<Field>>& GetFields () const override;
};


class VULKANWRAPPER_API UBO final : public FieldContainer, public Noncopyable {
public:
    uint32_t                            binding;
    uint32_t                            descriptorSet;
    std::string                         name;
    std::vector<std::unique_ptr<Field>> fields;

    uint32_t GetFullSize () const;

    virtual const std::vector<std::unique_ptr<Field>>& GetFields () const override;
};


class VULKANWRAPPER_API Sampler final {
public:
    enum class Type {
        Sampler1D,
        Sampler2D,
        Sampler3D,
        SamplerCube
    };

    std::string name;
    uint32_t    binding;
    uint32_t    descriptorSet;
    Type        type;
    uint32_t    arraySize; // 0 for non-arrays
};


class VULKANWRAPPER_API Output {
public:
    std::string   name;
    uint32_t      location;
    FieldType type;
    uint32_t      arraySize; // 0 for non-arrays
};


class VULKANWRAPPER_API Input {
public:
    std::string   name;
    uint32_t      location;
    FieldType type;
    uint32_t      arraySize; // 0 for non-arrays
    uint32_t      sizeInBytes;
};


VULKANWRAPPER_API
std::vector<std::shared_ptr<UBO>> GetUBOsFromBinary (const std::vector<uint32_t>& binary);

VULKANWRAPPER_API
std::vector<Sampler> GetSamplersFromBinary (const std::vector<uint32_t>& binary);

VULKANWRAPPER_API
std::vector<Input> GetInputsFromBinary (const std::vector<uint32_t>& binary);

VULKANWRAPPER_API
std::vector<Output> GetOutputsFromBinary (const std::vector<uint32_t>& binary);

VULKANWRAPPER_API
VkFormat FieldTypeToVkFormat (FieldType fieldType);


} // namespace SR

} // namespace GVK

#endif