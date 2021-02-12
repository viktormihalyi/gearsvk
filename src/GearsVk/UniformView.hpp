#ifndef UNIFORM_VIEW_HPP
#define UNIFORM_VIEW_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"

#include "GearsVkAPI.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

namespace GVK {

USING_PTR (ShaderModule);

namespace SR {

USING_PTR (FieldContainer);
USING_PTR (Field);
USING_PTR (UBO);

// view (size and offset) to a single variable (could be primitive, struct, array) in a uniform block
// we can walk down the struct hierarchy with operator[](std::string_view)
// we can select an element in an array with operator[](uint32_t)
// we can set the variable with operator=(T)

USING_PTR (UView);
class GVK_RENDERER_API UView final {
public:
    static const UView invalidUview;

private:
    enum class Type {
        Variable,
        Array,
    };

    const Type                type;
    uint8_t*                  data;
    const uint32_t            offset;
    const uint32_t            size;
    const SR::FieldContainer& parentContainer;
    const SR::FieldU&         currentField;

public:
    UView (Type                      type,
           uint8_t*                  data,
           uint32_t                  offset,
           uint32_t                  size,
           const SR::FieldContainer& parentContainer,
           const SR::FieldU&         currentField = nullptr);

    UView (const Ptr<SR::UBO>& root, uint8_t* data);

    UView (const UView&);

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

    uint32_t GetOffset () const;

    UView operator[] (std::string_view str);

    UView operator[] (uint32_t index);

    std::vector<std::string> GetFieldNames () const;
};


// view to a single UBO + properly sized byte array for it
USING_PTR (IUData);
class GVK_RENDERER_API IUData {
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
class GVK_RENDERER_API DummyUData final : public IUData {
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
class GVK_RENDERER_API UDataExternal final : public IUData, public Noncopyable {
private:
    UView    root;
    uint8_t* bytes;
    uint32_t size;

public:
    UDataExternal (const Ptr<SR::UBO>& ubo, uint8_t* bytes, uint32_t size);

    virtual UView operator[] (std::string_view str) override;

    virtual std::vector<std::string> GetNames () override;

    virtual uint8_t* GetData () override;

    virtual uint32_t GetSize () const override;
};


USING_PTR (UDataInternal);
class GVK_RENDERER_API UDataInternal final : public IUData, public Noncopyable {
private:
    std::vector<uint8_t> bytes;
    UView                root;

public:
    UDataInternal (const Ptr<SR::UBO>& ubo);

    virtual UView operator[] (std::string_view str) override;

    virtual std::vector<std::string> GetNames () override;

    virtual uint8_t* GetData () override;

    virtual uint32_t GetSize () const override;
};


// view and byte array for _all_ UBOs in a shader
// we can select a single UBO with operator[](std::string_view)

USING_PTR (ShaderUData);
class GVK_RENDERER_API ShaderUData final : public Noncopyable {
private:
    std::vector<IUDataU>      udatas;
    std::vector<std::string>  uboNames;
    std::vector<Ptr<SR::UBO>> ubos;

public:
    ShaderUData (const std::vector<Ptr<SR::UBO>>& ubos);
    ShaderUData (const std::vector<uint32_t>& shaderBinary);
    ShaderUData (const ShaderModuleU& shaderModule);
    ShaderUData (const ShaderModule& shaderModule);

    IUData& operator[] (std::string_view str);

    Ptr<SR::UBO> GetUbo (std::string_view str);
};


} // namespace SR

}


#endif