#ifndef SHADERREFLECTION_HPP
#define SHADERREFLECTION_HPP

#include "GearsVkAPI.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>


// NOTE: SSBOs are not supported


namespace SR {


// clang-format off

enum class FieldType {
    // Scalars
    Bool,
    Int,
    Uint,
    Float,
    Double,

    // vectors
    Bvec2, Ivec2, Uvec2, Vec2, Dvec2,
    Bvec3, Ivec3, Uvec3, Vec3, Dvec3,
    Bvec4, Ivec4, Uvec4, Vec4, Dvec4,

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

GEARSVK_API
std::string FieldTypeToString (FieldType fieldType);


USING_PTR (Field);

USING_PTR (FieldProvider);
class FieldProvider {
public:
    virtual std::vector<SR::FieldP> GetFields () const = 0;
};


USING_PTR (Field);
class Field final : public FieldProvider, public Noncopyable {
public:
    USING_CREATE (Field);

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

    std::vector<FieldP> structFields; // when type == FieldType::Struct

    Field ();

    bool IsArray () const;

    bool IsStruct () const;

    uint32_t GetSize () const;

    virtual std::vector<SR::FieldP> GetFields () const override;
};


USING_PTR (UBO);
class GEARSVK_API UBO final : public FieldProvider, public Noncopyable {
    USING_CREATE (UBO);

public:
    uint32_t            binding;
    uint32_t            descriptorSet;
    std::string         name;
    std::vector<FieldP> fields;

    uint32_t GetFullSize () const;

    virtual std::vector<SR::FieldP> GetFields () const override;
};


class GEARSVK_API Sampler final {
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


class GEARSVK_API Output {
public:
    std::string   name;
    uint32_t      location;
    SR::FieldType type;
    uint32_t      arraySize; // 0 for non-arrays
};


class GEARSVK_API Input {
public:
    std::string   name;
    uint32_t      location;
    SR::FieldType type;
    uint32_t      arraySize; // 0 for non-arrays
    uint32_t      sizeInBytes;
};


GEARSVK_API
std::vector<UBOP> GetUBOsFromBinary (const std::vector<uint32_t>& binary);

GEARSVK_API
std::vector<Sampler> GetSamplersFromBinary (const std::vector<uint32_t>& binary);

GEARSVK_API
std::vector<Input> GetInputsFromBinary (const std::vector<uint32_t>& binary);

GEARSVK_API
std::vector<Output> GetOutputsFromBinary (const std::vector<uint32_t>& binary);

GEARSVK_API
VkFormat FieldTypeToVkFormat (FieldType fieldType);


} // namespace SR

#endif