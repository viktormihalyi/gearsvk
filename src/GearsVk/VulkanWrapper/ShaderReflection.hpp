#ifndef SHADERREFLECTION_HPP
#define SHADERREFLECTION_HPP

#include "Assert.hpp"

#include <cstdint>
#include <iostream>
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

    USING_PTR (Field);

    struct Field final {
        std::string name;
        FieldType   type;

        // with respect to parent struct
        uint32_t offset;

        // 0 for structs
        // single element size for arrays
        uint32_t size;

        uint32_t arraySize;   // 0 for non-arrays
        uint32_t arrayStride; // 0 for non-arrays

        std::vector<FieldP> structFields; // when type == FieldType::Struct

        USING_CREATE (Field);

        Field ()
            : name ("")
            , type (FieldType::Unknown)
            , offset (0)
            , size (0)
            , arraySize (0)
            , arrayStride (0)
        {
        }

        bool IsArray () const
        {
            return arraySize > 0;
        }

        bool IsStruct () const
        {
            return type == FieldType::Struct;
        }

        uint32_t GetSize () const
        {
            if (IsArray ()) {
                return arrayStride * arraySize;
            }

            if (IsStruct ()) {
                if (ERROR (structFields.empty ())) {
                    return 0;
                }

                const Field& lastField = *structFields[structFields.size () - 1];
                return lastField.offset + lastField.GetSize ();
            }   
            
            return size;
        }
    };

    uint32_t            binding;
    uint32_t            descriptorSet;
    std::string         name;
    std::vector<FieldP> fields;

    UBO ()
        : binding ()
        , name ("")
    {
    }

    uint32_t GetFullSize () const
    {
        if (ERROR (fields.empty ())) {
            return 0;
        }

        const Field& lastField = *fields[fields.size () - 1];
        return lastField.offset + lastField.GetSize ();
    }

    bool operator== (const UBO& other) const
    {
        return binding == other.binding && descriptorSet == other.descriptorSet;
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