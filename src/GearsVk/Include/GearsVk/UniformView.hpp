#ifndef UNIFORM_VIEW_HPP
#define UNIFORM_VIEW_HPP

#include "Utils/Assert.hpp"
#include "Utils/Noncopyable.hpp"
#include <memory>

#include "GearsVkAPI.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

#include "spdlog/spdlog.h"

namespace GVK {

class ShaderModule;

namespace SR {

class FieldContainer;
class Field;
class UBO;

// view (size and offset) to a single variable (could be primitive, struct, array) in a uniform block
// we can walk down the struct hierarchy with operator[](std::string_view)
// we can select an element in an array with operator[](uint32_t)
// we can set the variable with operator=(T)

class GVK_RENDERER_API UView final {
public:
    static const UView invalidUview;

private:
    enum class Type {
        Variable,
        Array,
    };

    const Type                        type;
    uint8_t*                          data;
    const uint32_t                    offset;
    const uint32_t                    size;
    const FieldContainer&         parentContainer;
    const std::unique_ptr<Field>& currentField;

public:
    UView (Type                              type,
           uint8_t*                          data,
           uint32_t                          offset,
           uint32_t                          size,
           const FieldContainer&         parentContainer,
           const std::unique_ptr<Field>& currentField = nullptr);

    UView (const std::shared_ptr<UBO>& root, uint8_t* data);

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

        memcpy (data + offset, &other, size);
    }

    uint32_t GetOffset () const;

    UView operator[] (std::string_view str);

    UView operator[] (uint32_t index);

    std::vector<std::string> GetFieldNames () const;
};


// view to a single UBO + properly sized byte array for it
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


class GVK_RENDERER_API UDataExternal final : public IUData, public Noncopyable {
private:
    UView    root;
    uint8_t* bytes;
    uint32_t size;

public:
    UDataExternal (const std::shared_ptr<UBO>& ubo, uint8_t* bytes, uint32_t size);

    virtual UView operator[] (std::string_view str) override;

    virtual std::vector<std::string> GetNames () override;

    virtual uint8_t* GetData () override;

    virtual uint32_t GetSize () const override;
};


class GVK_RENDERER_API UDataInternal final : public IUData, public Noncopyable {
private:
    std::vector<uint8_t> bytes;
    UView                root;

public:
    UDataInternal (const std::shared_ptr<UBO>& ubo);

    virtual UView operator[] (std::string_view str) override;

    virtual std::vector<std::string> GetNames () override;

    virtual uint8_t* GetData () override;

    virtual uint32_t GetSize () const override;
};


// view and byte array for _all_ UBOs in a shader
// we can select a single UBO with operator[](std::string_view)

class GVK_RENDERER_API ShaderUData final : public Noncopyable {
private:
    std::vector<std::unique_ptr<IUData>>  udatas;
    std::vector<std::string>              uboNames;
    std::vector<std::shared_ptr<UBO>> ubos;

public:
    ShaderUData (const std::vector<std::shared_ptr<UBO>>& ubos);
    ShaderUData (const std::vector<uint32_t>& shaderBinary);
    ShaderUData (const std::unique_ptr<ShaderModule>& shaderModule);
    ShaderUData (const ShaderModule& shaderModule);

    IUData& operator[] (std::string_view str);

    std::shared_ptr<UBO> GetUbo (std::string_view str);
};


} // namespace SR

} // namespace GVK


#endif

