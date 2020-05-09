#ifndef UNIFORMBLOCK_HPP
#define UNIFORMBLOCK_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "ShaderReflection.hpp"

#include <cmath>
#include <cstdint>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

struct ShaderType {
    uint32_t                size;
    uint32_t                alignment;
    std::string             glslType;
    std::optional<uint32_t> arraySize;
};

namespace ST {

static ShaderType uint {1 * sizeof (uint32_t), 4, "uint"};

static ShaderType vec1 {1 * sizeof (float), 4, "float"};
static ShaderType vec2 {2 * sizeof (float), 8, "vec2"};
static ShaderType vec3 {3 * sizeof (float), 16, "vec3"};
static ShaderType vec4 {4 * sizeof (float), 16, "vec4"};

static ShaderType mat3 {3 * 3 * sizeof (float), 16, "mat3"};
static ShaderType mat4 {4 * 4 * sizeof (float), 16, "mat4"};


template<uint32_t SIZE>
static ShaderType vec1Array {4 * SIZE, 32, "vec1", SIZE};
template<uint32_t SIZE>
static ShaderType vec2Array {8 * SIZE, 32, "vec2", SIZE};
template<uint32_t SIZE>
static ShaderType vec3Array {12 * SIZE, 32, "vec3", SIZE};
template<uint32_t SIZE>
static ShaderType vec4Array {16 * SIZE, 32, "vec4", SIZE};

} // namespace ST


class ShaderStruct {
private:
    std::string name;

    uint32_t fullSize;

    std::unordered_map<std::string, uint32_t>                  nameMapping;
    std::vector<std::tuple<std::string, ShaderType, uint32_t>> variables;

public:
    USING_PTR (ShaderStruct);
    ShaderStruct (const std::string& name, const std::vector<std::pair<std::string, ShaderType>>& types)
        : name (name)
    {
        uint32_t offset = 0;
        for (auto& s : types) {
            offset = static_cast<uint32_t> (std::ceil (static_cast<float> (offset) / s.second.alignment)) * s.second.alignment;
            variables.emplace_back (s.first, s.second, offset);
            nameMapping[s.first] = variables.size () - 1;
            offset += s.second.size;
        }
        fullSize = offset;
    }

    ShaderStruct (const std::vector<std::pair<std::string, ShaderType>>& types)
        : ShaderStruct ("", types)
    {
    }

    ShaderStruct (const SR::UBO& reflectedUBO)
        : name (reflectedUBO.name)
        , fullSize (reflectedUBO.GetFullSize ())
    {
        for (const auto& f : reflectedUBO.fields) {
            variables.emplace_back (f.name, ShaderType {f.size, 0}, f.offset);
            nameMapping[f.name] = variables.size () - 1;
        }
    }

    uint32_t GetVariableIndex (const std::string& name) const
    {
        return nameMapping.at (name);
    }

    uint32_t GetOffset (const std::string& name) const
    {
        return std::get<2> (variables[GetVariableIndex (name)]);
    }

    uint32_t GetSize (const std::string& name) const
    {
        return std::get<1> (variables[GetVariableIndex (name)]).size;
    }

    uint32_t GetFullSize () const
    {
        return fullSize;
    }

    std::string GetName () const
    {
        return name;
    }

    bool HasUniform (const std::string& name) const
    {
        return nameMapping.find (name) != nameMapping.end ();
    }
};


class UniformValue {
public:
    size_t size;
    void*  value;

    template<typename T>
    T& As ()
    {
        return *reinterpret_cast<T*> (value);
    }

    template<typename T>
    void operator= (const T& other)
    {
        if (ASSERT (size == sizeof (T))) {
            *reinterpret_cast<T*> (value) = other;
        }
    }
};


class UniformBlock : public Noncopyable {
public:
    const ShaderStruct structType;

    std::vector<std::vector<uint8_t>> data;

    uint32_t    binding;
    std::string variableName;

public:
    USING_PTR (UniformBlock);

    UniformBlock (uint32_t binding, const std::string& variableName, const ShaderStruct& structType)
        : binding (binding)
        , variableName (variableName)
        , structType (structType)
    {
        data.resize (1);
        data[0].resize (structType.GetFullSize (), 0);
    }

    UniformBlock (const std::vector<std::pair<std::string, ShaderType>>& types)
        : UniformBlock (0, "", types)
    {
    }

    uint32_t GetSize () const { return structType.GetFullSize (); }

    const void* GetData () const { return data[0].data (); }

    template<typename T>
    void Set (const std::string& name, const T& value)
    {
        GetRef<T> (name) = value;
    }

    void* GetRawPtr (const std::string& name)
    {
        if (ERROR (!structType.HasUniform (name))) {
            throw std::runtime_error ("no uniform name '" + name + "'");
        }

        const uint32_t offset = structType.GetOffset (name);
        return reinterpret_cast<void*> (&data[0][offset]);
    }

    template<typename ReturnType>
    ReturnType* GetPtr (const std::string& name)
    {
        if (ERROR (!structType.HasUniform (name))) {
            throw std::runtime_error ("no uniform name '" + name + "'");
        }

        ASSERT (sizeof (ReturnType) == structType.GetSize (name));

        const uint32_t offset = structType.GetOffset (name);
        return reinterpret_cast<ReturnType*> (&data[0][offset]);
    }

    template<typename ReturnType>
    ReturnType GetValue (const std::string& name)
    {
        return *GetPtr<ReturnType> (name);
    }

    template<typename ReturnType>
    ReturnType& GetRef (const std::string& name)
    {
        return *GetPtr<ReturnType> (name);
    }

    UniformValue operator[] (const std::string& name)
    {
        return {structType.GetSize (name), GetRawPtr (name)};
    }
};


struct ShaderBlocks {
    std::vector<UniformBlock::P>              blocks;
    std::unordered_map<std::string, uint32_t> nameMapping;

    USING_PTR (ShaderBlocks);

    void AddBlock (const UniformBlock::P& block)
    {
        std::string name = block->variableName;
        blocks.push_back (block);
        nameMapping[name] = blocks.size () - 1;
    }

    void Clear ()
    {
        blocks.clear ();
        nameMapping.clear ();
    }

    UniformBlock& GetBlock (const std::string& name)
    {
        return *blocks[nameMapping[name]];
    }

    UniformBlock& operator[] (const std::string& name)
    {
        return GetBlock (name);
    }
};

#endif