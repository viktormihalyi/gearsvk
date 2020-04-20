#ifndef UNIFORMBLOCK_HPP
#define UNIFORMBLOCK_HPP

#include "Assert.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct ShaderType {
    uint32_t size;
    uint32_t alignment;
};

namespace ST {

static ShaderType vec1 {1 * sizeof (float), 4};
static ShaderType vec2 {2 * sizeof (float), 8};
static ShaderType vec3 {3 * sizeof (float), 16};
static ShaderType vec4 {4 * sizeof (float), 16};

static ShaderType mat3 {3 * 3 * sizeof (float), 16};
static ShaderType mat4 {4 * 4 * sizeof (float), 16};


template<uint32_t SIZE>
static ShaderType vec1Array {4 * SIZE, 32};
template<uint32_t SIZE>
static ShaderType vec2Array {8 * SIZE, 32};
template<uint32_t SIZE>
static ShaderType vec3Array {12 * SIZE, 32};
template<uint32_t SIZE>
static ShaderType vec4Array {16 * SIZE, 32};

} // namespace ST


class UniformBlock {
private:
    uint32_t fullSize;

    std::map<std::string, uint32_t>                            nameMapping;
    std::vector<std::tuple<std::string, ShaderType, uint32_t>> variables;

    std::vector<uint8_t> data;

public:
    UniformBlock (const std::vector<std::pair<std::string, ShaderType>>& types)
    {
        std::set<std::string> uniqueNames;
        for (const auto& a : types) {
            uniqueNames.insert (a.first);
        }

        ASSERT (uniqueNames.size () == types.size ());

        uint32_t offset = 0;
        for (auto& s : types) {
            offset = std::ceil (static_cast<float> (offset) / s.second.alignment) * s.second.alignment;
            variables.emplace_back (s.first, s.second, offset);
            nameMapping[s.first] = variables.size () - 1;
            offset += s.second.size;
        }
        fullSize = offset;

        data.resize (fullSize, 0);
    }

    uint32_t GetSize () const { return fullSize; }

    uint32_t GetOffset (const std::string& name)
    {
        const uint32_t variableIndex = nameMapping[name];
        return std::get<2> (variables[variableIndex]);
    }

    const void* GetData () const { return data.data (); }

    template<typename T>
    void Set (const std::string& name, const T& value)
    {
        if (ERROR (nameMapping.find (name) == nameMapping.end ())) {
            return;
        }

        const uint32_t variableIndex = nameMapping[name];

        const uint32_t variableSize = std::get<1> (variables[variableIndex]).size;
        ASSERT (sizeof (T) == variableSize);

        const uint32_t offset = std::get<2> (variables[variableIndex]);

        T* typedData = reinterpret_cast<T*> (&data[offset]);

        *typedData = value;
    }

    template<typename ReturnType>
    ReturnType Get (const std::string& name)
    {
        const uint32_t variableIndex = nameMapping[name];

        const uint32_t variableSize = std::get<1> (variables[variableIndex]).size;

        if constexpr (!std::is_pointer_v<ReturnType>) {
            ASSERT (sizeof (ReturnType) == variableSize);
        }

        const uint32_t offset = std::get<2> (variables[variableIndex]);

        if constexpr (std::is_pointer_v<ReturnType>) {
            return reinterpret_cast<ReturnType> (&data[offset]);
        } else {
            return *reinterpret_cast<ReturnType*> (&data[offset]);
        }
    }

    template<typename ReturnType>
    ReturnType& GetRef (const std::string& name)
    {
        return *Get<ReturnType*> (name);
    }
};

#endif