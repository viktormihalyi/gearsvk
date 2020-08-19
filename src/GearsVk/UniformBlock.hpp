#ifndef UNIFORMBLOCK_HPP
#define UNIFORMBLOCK_HPP

#include "Assert.hpp"
#include "Dummy.hpp"
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


// represents one uniform buffer object _type_
USING_PTR (ShaderStruct);
class ShaderStruct {
private:
    uint32_t                                                   fullSize;
    std::unordered_map<std::string, uint32_t>                  nameMapping;
    std::vector<std::tuple<std::string, ShaderType, uint32_t>> variables;

public:
    USING_CREATE (ShaderStruct);
    ShaderStruct (const std::vector<std::pair<std::string, ShaderType>>& types)
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


    ShaderStruct (const SR::UBO& reflectedUBO)
        : fullSize (reflectedUBO.GetFullSize ())
    {
        for (const auto& f : reflectedUBO.fields) {
            variables.emplace_back (f->name, ShaderType {f->size, 0}, f->offset);
            nameMapping[f->name] = variables.size () - 1;
        }
    }

    // intentionally empty
    ShaderStruct (std::nullptr_t)
    {
        fullSize = 0;
    }

    uint32_t GetVariableIndex (const std::string& name) const
    {
        return nameMapping.at (name);
    }

    bool ContainsName (const std::string& name) const
    {
        return nameMapping.find (name) != nameMapping.end ();
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

    bool HasUniform (const std::string& name) const
    {
        return nameMapping.find (name) != nameMapping.end ();
    }
};


class UniformView {
private:
    const size_t size;
    void* const  value;
    const bool   valid;

public:
    UniformView (size_t size, void* value, bool valid)
        : size (size)
        , value (value)
        , valid (valid)
    {
    }

    template<typename T>
    T& As ()
    {
        if (GVK_ERROR (!valid)) {
            return dummy<T>;
        }

        return *reinterpret_cast<T*> (value);
    }

    template<typename T>
    void operator= (const T& other)
    {
        if (GVK_ERROR (!valid)) {
            return;
        }

        if (GVK_ASSERT (size == sizeof (T))) {
            *reinterpret_cast<T*> (value) = other;
        }
    }
};

using UniformVariableView = UniformView;
using UniformBlockView    = UniformView;


// represents one uniform buffer object in one shader
USING_PTR (UniformBlock);
class UniformBlock : public Noncopyable {
public:
    friend class ShaderBlocks;

private:
    const ShaderStruct structType;

    std::vector<uint8_t> data;

    uint32_t    binding;
    std::string variableName;

public:
    USING_CREATE (UniformBlock);

    UniformBlock (uint32_t binding, const std::string& variableName, const ShaderStruct& structType)
        : binding (binding)
        , variableName (variableName)
        , structType (structType)
    {
        data.resize (structType.GetFullSize (), 0);
    }

    // intentionally empty
    UniformBlock (std::nullptr_t)
        : UniformBlock (0, "", ShaderStruct (nullptr))
    {
    }

    uint32_t GetSize () const
    {
        return structType.GetFullSize ();
    }

    void* GetData ()
    {
        return data.data ();
    }

    void* GetPtrTo (const std::string& name)
    {
        if (GVK_ERROR (!structType.HasUniform (name))) {
            throw std::runtime_error ("no uniform name '" + name + "'");
        }

        const uint32_t offset = structType.GetOffset (name);
        return reinterpret_cast<void*> (&data[offset]);
    }

    UniformVariableView operator[] (const std::string& name)
    {
        if (!structType.ContainsName (name)) {
            return UniformVariableView (0, nullptr, false);
        }

        return UniformVariableView (structType.GetSize (name), GetPtrTo (name), true);
    }

    UniformBlockView GetView ()
    {
        return UniformBlockView (GetSize (), GetData (), true);
    }

    template<typename T>
    struct is_array_or_vector {
        enum { value = false };
    };

    template<typename T, typename A>
    struct is_array_or_vector<std::vector<T, A>> {
        enum { value = true };
    };

    template<typename T, std::size_t N>
    struct is_array_or_vector<std::array<T, N>> {
        enum { value = true };
    };

    template<typename T>
    void operator= (const std::vector<T>& other)
    {
        const uint32_t uniformVariableSize = data.size ();
        const uint32_t arraySize           = other.size ();
        const uint32_t arrayElementSize    = sizeof (T);
        const uint32_t arrayFullSize       = arrayElementSize * arraySize;

        GVK_ASSERT (data.size () == GetSize ());

        if (GVK_ASSERT (data.size () == sizeof (T) * other.size ())) {
            memcpy (data.data (), other.data (), sizeof (T) * other.size ());
        }
    }

    template<typename T>
    void operator= (const T& other)
    {
        GVK_ASSERT (data.size () == GetSize ());

        if (GVK_ASSERT (data.size () == sizeof (other))) {
            memcpy (data.data (), &other, sizeof (T));
        }
    }

    template<typename T, size_t N>
    void operator!= (const std::array<T, N>& other)
    {
        memcpy (data.data (), other.data (), sizeof (T) * other.size ());
    }

#if 0
    template<typename T>
    void Set (const std::string& name, const T& value)
    {
        GetRef<T> (name) = value;
    }

    template<typename ReturnType>
    ReturnType* GetPtr (const std::string& name)
    {
        if (GVK_ERROR (!structType.HasUniform (name))) {
            throw std::runtime_error ("no uniform name '" + name + "'");
        }

        GVK_ASSERT (sizeof (ReturnType) == structType.GetSize (name));

        const uint32_t offset = structType.GetOffset (name);
        return reinterpret_cast<ReturnType*> (&data[offset]);
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
#endif
};


// contains all uniform buffer objects for a single shader
USING_PTR (ShaderBlocks);
class ShaderBlocks {
private:
    UniformBlock dummy;

    std::vector<UniformBlockP>                blocks;
    std::unordered_map<std::string, uint32_t> nameMapping;

public:
    USING_CREATE (ShaderBlocks);

    ShaderBlocks ()
        : dummy (nullptr)
    {
    }

    void AddBlock (const UniformBlockP& block)
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
        if (nameMapping.find (name) == nameMapping.end ()) {
            return dummy;
        }

        return *blocks[nameMapping[name]];
    }

    UniformBlock& operator[] (const std::string& name)
    {
        return GetBlock (name);
    }
};

#endif