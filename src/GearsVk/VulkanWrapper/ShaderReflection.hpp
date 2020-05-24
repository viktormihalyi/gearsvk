#ifndef SHADERREFLECTION_HPP
#define SHADERREFLECTION_HPP

#include "Assert.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace SR {

struct GEARSVK_API UBO {
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
        Mat2, Mat3, Mat4,
        Mat2x2, Mat2x3, Mat2x4,
        Mat3x2, Mat3x3, Mat3x4,
        Mat4x2, Mat4x3, Mat4x4,

        // double matrices
        Dmat2, Dmat3, Dmat4,
        Dmat2x2, Dmat2x3, Dmat2x4,
        Dmat3x2, Dmat3x3, Dmat3x4,
        Dmat4x2, Dmat4x3, Dmat4x4,

        // ...
        Struct,
        Unknown
    };

    // clang-format on

    struct Field {
        std::string name;
        uint32_t    offset;
        uint32_t    size;
        FieldType   type;
    };

    uint32_t           binding;
    std::string        name;
    std::vector<Field> fields;

    uint32_t GetFullSize () const
    {
        if (fields.empty ()) {
            return 0;
        }

        const Field& lastField = fields[fields.size () - 1];
        ASSERT (lastField.size != 0);

        return lastField.offset + lastField.size;
    }

    bool operator== (const UBO& other) const
    {
        return binding == other.binding;
    }
};


struct GEARSVK_API Sampler {
    enum class Type {
        Sampler1D,
        Sampler2D,
        Sampler3D,
        SamplerCube
    };
    std::string name;
    uint32_t    binding;
    Type        type;

    bool operator== (const Sampler& other) const
    {
        return binding == other.binding;
    }
};

} // namespace SR

#endif