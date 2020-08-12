#ifndef UNIFORM_VIEW_HPP
#define UNIFORM_VIEW_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "ShaderModule.hpp"
#include "ShaderReflection.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

namespace SR {

// view (size and offset) to a single variable (could be primitive, struct, array) in a uniform block
// we can walk down the struct hierarchy with operator[](std::string_view)
// we can select an element in an array with operator[](uint32_t)
// we can set the variable with operator=(T)

USING_PTR (UView);
class UView final {
private:
    enum class Type {
        Variable,
        Array,
    };

    const Type               type;
    uint8_t*                 data;
    const uint32_t           offset;
    const uint32_t           size;
    const SR::FieldProviderP parentContainer;
    const SR::FieldP         currentField;

public:
    USING_CREATE (UView);
    UView (Type                      type,
           uint8_t*                  data,
           uint32_t                  offset,
           uint32_t                  size,
           const SR::FieldProviderP& parentContainer,
           const SR::FieldP&         currentField = nullptr)
        : type (type)
        , data (data)
        , offset (offset)
        , size (size)
        , parentContainer (parentContainer)
        , currentField (currentField)
    {
    }

    UView (const SR::UBOP& root, uint8_t* data)
        : UView (Type::Variable, data, 0, root->GetFullSize (), root, nullptr)
    {
    }

    template<typename T>
    void operator= (T& other)
    {
        ASSERT (type == Type::Variable);
        ASSERT (sizeof (T) == size);

#ifndef NDEBUG
        std::cout << "setting uniform \"" << currentField->name << "\" (type: " << SR::FieldTypeToString (currentField->type)
                  << ", size: " << currentField->size
                  << ", offset: " << offset << ")"
                  << " with " << sizeof (T) << " bytes of data (type: " << typeid (other).name () << ")" << std::endl;
#endif
        memcpy (data + offset, &other, size);
    }

    uint32_t GetOffset () const
    {
        return offset;
    }

    UView operator[] (std::string_view str)
    {
        ASSERT (type != Type::Array);
        ASSERT (parentContainer != nullptr);

        for (auto& f : parentContainer->GetFields ()) {
            if (str == f->name) {
                if (f->IsArray ()) {
                    return UView (Type::Array, data, offset + f->offset, f->size, f, f);
                } else {
                    return UView (Type::Variable, data, offset + f->offset, f->size, f, f);
                }
            }
        }
    }

    UView operator[] (uint32_t index)
    {
        ASSERT (type == Type::Array);
        ASSERT (currentField != nullptr);
        ASSERT (currentField->IsArray ());
        ASSERT (index < currentField->arraySize);

        return UView (Type::Variable, data, index * currentField->arrayStride, size, parentContainer, currentField);
    }
};


// view to a single UBO + properly sized byte array for it
USING_PTR (IUData);
class IUData {
public:
    virtual ~IUData () = default;

    virtual UView    operator[] (std::string_view str) = 0;
    virtual uint8_t* GetData ()                        = 0;
    virtual uint32_t GetSize () const                  = 0;
};


USING_PTR (UDataExternal);
class UDataExternal final : public IUData, public Noncopyable {
private:
    UView    root;
    uint8_t* bytes;
    uint32_t size;

public:
    USING_CREATE (UDataExternal);
    UDataExternal (const SR::UBOP& ubo, uint8_t* bytes, uint32_t size)
        : root (ubo, bytes)
        , bytes (bytes)
        , size (size)
    {
        ASSERT (ubo->GetFullSize () == size);
        memset (bytes, 0, size);
    }

    UView operator[] (std::string_view str)
    {
        return root[str];
    }

    uint8_t* GetData ()
    {
        return bytes;
    }

    uint32_t GetSize () const
    {
        return size;
    }
};


USING_PTR (UDataInternal);
class UDataInternal final : public IUData, public Noncopyable {
private:
    std::vector<uint8_t> bytes;
    UView                root;

public:
    USING_CREATE (UDataInternal);
    UDataInternal (const SR::UBOP& ubo)
        : bytes (ubo->GetFullSize (), 0)
        , root (ubo, bytes.data ())
    {
        bytes.resize (ubo->GetFullSize ());
        std::fill (bytes.begin (), bytes.end (), 0);
    }

    UView operator[] (std::string_view str)
    {
        return root[str];
    }

    uint8_t* GetData ()
    {
        return bytes.data ();
    }

    uint32_t GetSize () const
    {
        return bytes.size ();
    }
};


// view and byte array for _all_ UBOs in a shader
// we can select a single UBO with operator[](std::string_view)

USING_PTR (ShaderUData);
class ShaderUData final : public Noncopyable {
private:
    std::vector<IUDataU>     udatas;
    std::vector<std::string> uboNames;
    std::vector<SR::UBOP>    ubos;

public:
    USING_CREATE (ShaderUData);
    ShaderUData (const std::vector<SR::UBOP>& ubos)
        : ubos (ubos)
    {
        for (auto& a : ubos) {
            udatas.push_back (UDataInternal::Create (a));
            uboNames.push_back (a->name);
        }
    }

    ShaderUData (const std::vector<uint32_t>& shaderBinary)
        : ShaderUData (SR::GetUBOsFromBinary (shaderBinary))
    {
    }

    ShaderUData (const ShaderModuleU& shaderModule)
        : ShaderUData (shaderModule->GetBinary ())
    {
    }

    ShaderUData (const ShaderModule& shaderModule)
        : ShaderUData (shaderModule.GetBinary ())
    {
    }

    IUData& operator[] (std::string_view str)
    {
        const uint32_t index = std::distance (uboNames.begin (), std::find (uboNames.begin (), uboNames.end (), str));
        return *udatas[index];
    }

    SR::UBOP GetUbo (std::string_view str)
    {
        const uint32_t index = std::distance (uboNames.begin (), std::find (uboNames.begin (), uboNames.end (), str));
        return ubos[index];
    }
};

USING_PTR (ShaderPipelineUData);
struct ShaderPipelineUData {
    ShaderUData vert;
    ShaderUData frag;

    USING_CREATE (ShaderPipelineUData);
    ShaderPipelineUData (ShaderPipeline& pipeline)
        : vert (pipeline.vertexShader.shader)
        , frag (pipeline.fragmentShader.shader)
    {
    }
};

} // namespace SR

#endif