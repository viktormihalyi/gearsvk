#include "UniformView.hpp"

#include "VulkanWrapper/ShaderModule.hpp"
#include "VulkanWrapper/ShaderReflection.hpp"

#include <algorithm>

#include "spdlog/spdlog.h"

namespace GVK {

namespace SR {

static const std::vector<std::unique_ptr<Field>> emptyFieldVector;

class EmptyFieldContainer : public FieldContainer {
public:
    virtual const std::vector<std::unique_ptr<Field>>& GetFields () const { return emptyFieldVector; }
} emptyFields;

const UView UView::invalidUview (UView::Type::Variable, nullptr, 0, 0, emptyFields, nullptr);

DummyUData dummyUData;


UView::UView (Type                              type,
              uint8_t*                          data,
              uint32_t                          offset,
              uint32_t                          size,
              const FieldContainer&         parentContainer,
              const std::unique_ptr<Field>& currentField)
    : type (type)
    , data (data)
    , offset (offset)
    , size (size)
    , parentContainer (parentContainer)
    , currentField (currentField)
{
}


UView::UView (const std::shared_ptr<UBO>& root, uint8_t* data)
    : UView (Type::Variable, data, 0, root->GetFullSize (), *root, nullptr)
{
}

UView::UView (const UView& other)
    : type (other.type)
    , data (other.data)
    , offset (other.offset)
    , size (other.size)
    , parentContainer (other.parentContainer)
    , currentField (other.currentField)
{
}

uint32_t UView::GetOffset () const
{
    return offset;
}

UView UView::operator[] (std::string_view str)
{
    if (GVK_ERROR (data == nullptr)) {
        return invalidUview;
    }

    GVK_ASSERT (type != Type::Array);

    for (const std::unique_ptr<Field>& f : parentContainer.GetFields ()) {
        if (str == f->name) {
            if (f->IsArray ()) {
                return UView (Type::Array, data, offset + f->offset, f->size, *f, f);
            } else {
                return UView (Type::Variable, data, offset + f->offset, f->size, *f, f);
            }
        }
    }

    spdlog::error ("No \"{}\" uniform named on \"{}\".", str, currentField->name);

    GVK_ASSERT (false);
    return invalidUview;
}

UView UView::operator[] (uint32_t index)
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

std::vector<std::string> UView::GetFieldNames () const
{
    std::vector<std::string> result;

    for (auto& f : parentContainer.GetFields ()) {
        result.push_back (f->name);
    }

    return result;
}


UDataInternal::UDataInternal (const std::shared_ptr<UBO>& ubo)
    : bytes (ubo->GetFullSize (), 0)
    , root (ubo, bytes.data ())
{
    bytes.resize (ubo->GetFullSize ());
    std::fill (bytes.begin (), bytes.end (), 0);
}

UView UDataInternal::operator[] (std::string_view str)
{
    return root[str];
}

std::vector<std::string> UDataInternal::GetNames ()
{
    return root.GetFieldNames ();
}

uint8_t* UDataInternal::GetData ()
{
    return bytes.data ();
}

uint32_t UDataInternal::GetSize () const
{
    return bytes.size ();
}

UDataExternal::UDataExternal (const std::shared_ptr<UBO>& ubo, uint8_t* bytes, uint32_t size)
    : root (ubo, bytes)
    , bytes (bytes)
    , size (size)
{
    GVK_ASSERT (ubo->GetFullSize () == size);
    memset (bytes, 0, size);
}

UView UDataExternal::operator[] (std::string_view str)
{
    return root[str];
}

std::vector<std::string> UDataExternal::GetNames ()
{
    return root.GetFieldNames ();
}

uint8_t* UDataExternal::GetData ()
{
    return bytes;
}

uint32_t UDataExternal::GetSize () const
{
    return size;
}


ShaderUData::ShaderUData (const std::vector<std::shared_ptr<UBO>>& ubos)
    : ubos (ubos)
{
    for (auto& a : ubos) {
        udatas.push_back (std::make_unique<UDataInternal> (a));
        uboNames.push_back (a->name);
    }
}

ShaderUData::ShaderUData (const std::vector<uint32_t>& shaderBinary)
    : ShaderUData (GetUBOsFromBinary (shaderBinary))
{
}

ShaderUData::ShaderUData (const std::unique_ptr<ShaderModule>& shaderModule)
    : ShaderUData (shaderModule->GetBinary ())
{
}

ShaderUData::ShaderUData (const ShaderModule& shaderModule)
    : ShaderUData (shaderModule.GetBinary ())
{
}

IUData& ShaderUData::operator[] (std::string_view str)
{
    const uint32_t index = std::distance (uboNames.begin (), std::find (uboNames.begin (), uboNames.end (), str));
    return *udatas[index];
}

std::shared_ptr<UBO> ShaderUData::GetUbo (std::string_view str)
{
    const uint32_t index = std::distance (uboNames.begin (), std::find (uboNames.begin (), uboNames.end (), str));
    return ubos[index];
}

} // namespace SR

} // namespace GVK
