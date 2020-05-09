#ifndef SHADERREFLECTION_HPP
#define SHADERREFLECTION_HPP

#include "Assert.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace SR {

struct UBO {
    struct Field {
        std::string name;
        uint32_t    offset;
        uint32_t    size;
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


struct Sampler {
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