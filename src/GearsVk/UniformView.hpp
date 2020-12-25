#ifndef UNIFORM_VIEW_HPP
#define UNIFORM_VIEW_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "ShaderModule.hpp"
#include "ShaderPipeline.hpp"
#include "ShaderReflection.hpp"

#include "GearsVkAPI.hpp"

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
class GEARSVK_API UView final {
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
    static const UView invalidUview;

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
    void operator= (const T& other)
    {
        static_assert (sizeof (T) >= 4, "there are no data types in glsl with less than 4 bytes");

        if (GVK_ERROR (data == nullptr)) {
            return;
        }

        GVK_ASSERT (type == Type::Variable);
        GVK_ASSERT (sizeof (T) == size);

#if 0
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
        if (GVK_ERROR (data == nullptr)) {
            return invalidUview;
        }

        GVK_ASSERT (type != Type::Array);
        GVK_ASSERT (parentContainer != nullptr);

        for (auto& f : parentContainer->GetFields ()) {
            if (str == f->name) {
                if (f->IsArray ()) {
                    return UView (Type::Array, data, offset + f->offset, f->size, f, f);
                } else {
                    return UView (Type::Variable, data, offset + f->offset, f->size, f, f);
                }
            }
        }

        GVK_ASSERT (false);
        return invalidUview;
    }

    UView operator[] (uint32_t index)
    {
        if (GVK_ERROR (data == nullptr)) {
            return invalidUview;
        }

        GVK_ASSERT (type == Type::Array);
        GVK_ASSERT (currentField != nullptr);
        GVK_ASSERT (currentField->IsArray ());
        GVK_ASSERT (index < currentField->arraySize);

        return UView (Type::Variable, data, index * currentField->arrayStride, size, parentContainer, currentField);
    }

    std::vector<std::string> GetFieldNames () const
    {
        if (GVK_ERROR (parentContainer != nullptr)) {
            return {};
        }

        std::vector<std::string> result;

        for (auto& f : parentContainer->GetFields ()) {
            result.push_back (f->name);
        }

        return result;
    }
};


// view to a single UBO + properly sized byte array for it
USING_PTR (IUData);
class GEARSVK_API IUData {
public:
    virtual ~IUData () = default;

    template<typename T>
    void operator= (const T& other)
    {
        static_assert (sizeof (T) >= 4, "there are no data types in glsl with less than 4 bytes");

        if (GetData () == nullptr) {
            GVK_ASSERT (false);
            return;
        }

        GVK_ASSERT (GetSize () == sizeof (T));
        memcpy (GetData (), &other, GetSize ());
    }

    template<typename T>
    void operator= (const std::vector<T>& other)
    {
        static_assert (sizeof (T) >= 4, "there are no data types in glsl with less than 4 bytes");

        if (GetData () == nullptr) {
            GVK_ASSERT (false);
            return;
        }

        GVK_ASSERT (sizeof (T) * other.size () == GetSize ());
        memcpy (GetData (), other.data (), GetSize ());
    }

    bool IsAllZero ()
    {
        if (GetData () == nullptr) {
            GVK_ASSERT (false);
            return true;
        }

        const uint8_t* dataPtr = GetData ();
        const uint32_t size    = GetSize ();

        for (uint32_t byteIndex = 0; byteIndex < size; ++byteIndex) {
            if (dataPtr[byteIndex] != 0) {
                return false;
            }
        }

        return true;
    }

    virtual UView                    operator[] (std::string_view str) = 0;
    virtual std::vector<std::string> GetNames ()                       = 0;
    virtual uint8_t*                 GetData ()                        = 0;
    virtual uint32_t                 GetSize () const                  = 0;
};


// view to a single UBO + properly sized byte array for it
class GEARSVK_API DummyUData final : public IUData {
public:
    virtual UView operator[] (std::string_view str) override
    {
        return UView::invalidUview;
    }

    virtual std::vector<std::string> GetNames () override
    {
        return {};
    }

    virtual uint8_t* GetData () override
    {
        return nullptr;
    }

    virtual uint32_t GetSize () const override
    {
        return 0;
    }
};


extern DummyUData dummyUData;


USING_PTR (UDataExternal);
class GEARSVK_API UDataExternal final : public IUData, public Noncopyable {
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
        GVK_ASSERT (ubo->GetFullSize () == size);
        memset (bytes, 0, size);
    }

    virtual UView operator[] (std::string_view str) override
    {
        return root[str];
    }

    virtual std::vector<std::string> GetNames () override
    {
        return root.GetFieldNames ();
    }

    virtual uint8_t* GetData () override
    {
        return bytes;
    }

    virtual uint32_t GetSize () const override
    {
        return size;
    }
};


USING_PTR (UDataInternal);
class GEARSVK_API UDataInternal final : public IUData, public Noncopyable {
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

    virtual UView operator[] (std::string_view str) override
    {
        return root[str];
    }

    virtual std::vector<std::string> GetNames () override
    {
        return root.GetFieldNames ();
    }

    virtual uint8_t* GetData () override
    {
        return bytes.data ();
    }

    virtual uint32_t GetSize () const override
    {
        return bytes.size ();
    }
};


// view and byte array for _all_ UBOs in a shader
// we can select a single UBO with operator[](std::string_view)

USING_PTR (ShaderUData);
class GEARSVK_API ShaderUData final : public Noncopyable {
    USING_CREATE (ShaderUData);

private:
    std::vector<IUDataU>     udatas;
    std::vector<std::string> uboNames;
    std::vector<SR::UBOP>    ubos;

public:
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


} // namespace SR

#endif