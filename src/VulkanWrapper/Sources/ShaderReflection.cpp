#include "ShaderReflection.hpp"

#include "Utils/Assert.hpp"

#include <optional>
#include <sstream>

// from SPRIV-Cross
#include "spirv_cross.hpp"

namespace SR {


Field::Field ()
    : name ("")
    , type (FieldType::Unknown)
    , offset (0)
    , size (0)
    , arraySize (0)
    , arrayStride (0)
{
}


bool Field::IsArray () const
{
    return arraySize != 1;
}


bool Field::IsFixedSizeArray () const
{
    return arraySize > 1;
}


bool Field::IsStruct () const
{
    return type == FieldType::Struct;
}


uint32_t Field::GetSize () const
{
    if (IsArray () && !IsFixedSizeArray ()) {
        return 0;
    }

    if (IsArray () && IsFixedSizeArray ()) {
        return arrayStride * arraySize;
    }

    if (IsStruct ()) {
        if (GVK_ERROR (structFields.empty ())) {
            return 0;
        }

        const Field& lastField = *structFields[structFields.size () - 1];
        return lastField.offset + lastField.GetSize ();
    }

    return size;
}


const std::vector<std::unique_ptr<Field>>& Field::GetFields () const
{
    return structFields;
}


bool UBO::HasFixedSize () const
{
    if (GVK_ERROR (fields.empty ())) {
        return false;
    }

    // TODO is checking the last field enough?
    const Field& lastField = *fields[fields.size () - 1];
    if (lastField.IsArray () && !lastField.IsFixedSizeArray ())
        return false;

    return true;
}


uint32_t UBO::GetFullSize () const
{
    if (GVK_ERROR (fields.empty ())) {
        return 0;
    }

    const Field& lastField = *fields[fields.size () - 1];

    if (GVK_ERROR (lastField.IsArray () && !lastField.IsFixedSizeArray ())) {
        return 0;
    }

    return lastField.offset + lastField.GetSize ();
}


const std::vector<std::unique_ptr<Field>>& UBO::GetFields () const
{
    return fields;
}


class AllDecorations {
public:
    std::string             name;
    std::optional<uint32_t> ArrayStride;
    std::optional<uint32_t> Offset;
    std::optional<uint32_t> Binding;
    std::optional<uint32_t> DescriptorSet;
    std::optional<uint32_t> Location;
    std::optional<uint32_t> InputAttachmentIndex;

private:
    static std::optional<uint32_t> GetDecor (const spirv_cross::Compiler& compiler, spirv_cross::ID resId, spv::Decoration decor)
    {
        if (compiler.has_decoration (resId, decor)) {
            return compiler.get_decoration (resId, decor);
        }

        return std::nullopt;
    }

    static std::optional<uint32_t> GetMemberDecor (const spirv_cross::Compiler& compiler, spirv_cross::ID resId, spv::Decoration decor, uint32_t idx)
    {
        if (compiler.has_member_decoration (resId, idx, decor)) {
            return compiler.get_member_decoration (resId, idx, decor);
        }

        return std::nullopt;
    }

    static std::string GetName (const spirv_cross::Compiler& compiler, spirv_cross::ID resId)
    {
        const std::string& name = compiler.get_name (resId);
        if (!name.empty ()) {
            return name;
        }

        return compiler.get_fallback_name (resId);
    }

#define AllDecorations_INITIALIZE_DECORATION(DecorName) \
    DecorName (GetDecor (compiler, resId, spv::Decoration##DecorName))

#define AllDecorations_INITIALIZE_MEMBER_DECORATION(DecorName) \
    DecorName (GetMemberDecor (compiler, resId, spv::Decoration##DecorName, memberIdx))

public:
    AllDecorations (const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type)
        : AllDecorations (compiler, type.self)
    {
    }

    AllDecorations (const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, uint32_t memberIdx)
        : AllDecorations (compiler, type.self, memberIdx)
    {
    }

    AllDecorations (const spirv_cross::Compiler& compiler, spirv_cross::ID resId)
        : name (GetName (compiler, resId))
        , AllDecorations_INITIALIZE_DECORATION (ArrayStride)
        , AllDecorations_INITIALIZE_DECORATION (Binding)
        , AllDecorations_INITIALIZE_DECORATION (DescriptorSet)
        , AllDecorations_INITIALIZE_DECORATION (Offset)
        , AllDecorations_INITIALIZE_DECORATION (Location)
        , AllDecorations_INITIALIZE_DECORATION (InputAttachmentIndex)
    {
    }

    AllDecorations (const spirv_cross::Compiler& compiler, spirv_cross::ID resId, uint32_t memberIdx)
        : name (compiler.get_member_name (resId, memberIdx))
        , AllDecorations_INITIALIZE_MEMBER_DECORATION (ArrayStride)
        , AllDecorations_INITIALIZE_MEMBER_DECORATION (Binding)
        , AllDecorations_INITIALIZE_MEMBER_DECORATION (DescriptorSet)
        , AllDecorations_INITIALIZE_MEMBER_DECORATION (Offset)
        , AllDecorations_INITIALIZE_MEMBER_DECORATION (InputAttachmentIndex)
    {
    }
};


static uint32_t BaseTypeToByteSize (spirv_cross::SPIRType::BaseType b)
{
    using BaseType = spirv_cross::SPIRType::BaseType;

    uint32_t baseTypeSize = 0;
    switch (b) {
        case BaseType::Boolean:
        case BaseType::Float:
        case BaseType::Int:
        case BaseType::UInt:
            return 4;

        case BaseType::Double:
            return 8;

        default:
            GVK_BREAK_STR ("bad BaseType");
            return 4;
    }
}


static uint32_t BaseTypeNMToByteSize (spirv_cross::SPIRType::BaseType b, uint32_t vecSize, uint32_t columns)
{
    return BaseTypeToByteSize (b) * vecSize * columns;
}


static FieldType BaseTypeNMToSRFieldType (spirv_cross::SPIRType::BaseType b, uint32_t vecSize, uint32_t columns)
{
    using BaseType = spirv_cross::SPIRType::BaseType;

    switch (b) {
        case BaseType::Float:
            switch (columns) {
                case 1:
                    switch (vecSize) {
                        case 1: return FieldType::Float;
                        case 2: return FieldType::Vec2;
                        case 3: return FieldType::Vec3;
                        case 4: return FieldType::Vec4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 2:
                    switch (vecSize) {
                        case 2: return FieldType::Mat2x2;
                        case 3: return FieldType::Mat2x3;
                        case 4: return FieldType::Mat2x4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 3:
                    switch (vecSize) {
                        case 2: return FieldType::Mat3x2;
                        case 3: return FieldType::Mat3x3;
                        case 4: return FieldType::Mat3x4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 4:
                    switch (vecSize) {
                        case 2: return FieldType::Mat4x2;
                        case 3: return FieldType::Mat4x3;
                        case 4: return FieldType::Mat4x4;
                        default: throw std::runtime_error ("wtf");
                    }
            }

        case BaseType::Double:
            switch (columns) {
                case 1:
                    switch (vecSize) {
                        case 1: return FieldType::Double;
                        case 2: return FieldType::Dvec2;
                        case 3: return FieldType::Dvec3;
                        case 4: return FieldType::Dvec4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 2:
                    switch (vecSize) {
                        case 2: return FieldType::Dmat2x2;
                        case 3: return FieldType::Dmat2x3;
                        case 4: return FieldType::Dmat2x4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 3:
                    switch (vecSize) {
                        case 2: return FieldType::Dmat3x2;
                        case 3: return FieldType::Dmat3x3;
                        case 4: return FieldType::Dmat3x4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 4:
                    switch (vecSize) {
                        case 2: return FieldType::Dmat4x2;
                        case 3: return FieldType::Dmat4x3;
                        case 4: return FieldType::Dmat4x4;
                        default: throw std::runtime_error ("wtf");
                    }
            }

        case BaseType::Boolean:
            switch (vecSize) {
                case 1: return FieldType::Bool;
                case 2: return FieldType::Bvec2;
                case 3: return FieldType::Bvec3;
                case 4: return FieldType::Bvec4;
                default: throw std::runtime_error ("wtf");
            }

        case BaseType::Int:
            switch (vecSize) {
                case 1: return FieldType::Int;
                case 2: return FieldType::Ivec2;
                case 3: return FieldType::Ivec3;
                case 4: return FieldType::Ivec4;
                default: throw std::runtime_error ("wtf");
            }

        case BaseType::UInt:
            switch (vecSize) {
                case 1: return FieldType::Uint;
                case 2: return FieldType::Uvec2;
                case 3: return FieldType::Uvec3;
                case 4: return FieldType::Uvec4;
                default: throw std::runtime_error ("wtf");
            }

        case BaseType::Struct:
            return FieldType::Struct;

        case BaseType::Image:
            return FieldType::Image;

        case BaseType::Int64:
            switch (vecSize) {
                case 1: return FieldType::i64;
                case 2: return FieldType::i64_vec2;
                case 3: return FieldType::i64_vec3;
                case 4: return FieldType::i64_vec4;
                default: throw std::runtime_error ("wtf");
            }

        case BaseType::UInt64:
            switch (vecSize) {
                case 1: return FieldType::u64;
                case 2: return FieldType::u64_vec2;
                case 3: return FieldType::u64_vec3;
                case 4: return FieldType::u64_vec4;
                default: throw std::runtime_error ("wtf");
            }

        default:
            GVK_BREAK ();
            return FieldType::Unknown;
    }
}


static void IterateTypeTree (spirv_cross::Compiler& compiler, spirv_cross::TypeID typeId, std::vector<std::unique_ptr<Field>>& parentFields, const uint32_t depth = 0)
{
    const spirv_cross::SPIRType& type = compiler.get_type (typeId);

    const uint32_t memberCount = type.member_types.size ();

    for (uint32_t i = 0; i < memberCount; ++i) {
        AllDecorations               typeMemDecorA (compiler, type.member_types[i]);
        AllDecorations               typeMemDecor (compiler, type.self, i);
        const spirv_cross::SPIRType& Mtype = compiler.get_type (type.member_types[i]);

        std::unique_ptr<Field> f = std::make_unique<Field> ();
        f->name                  = typeMemDecor.name;
        f->offset                = *typeMemDecor.Offset;
        f->arrayStride           = typeMemDecorA.ArrayStride ? *typeMemDecorA.ArrayStride : 0;
        f->arraySize             = !Mtype.array.empty () ? Mtype.array[0] : 1;
        f->size                  = (Mtype.width * Mtype.vecsize * Mtype.columns) / 8;
        f->type                  = BaseTypeNMToSRFieldType (Mtype.basetype, Mtype.vecsize, Mtype.columns);

        GVK_ASSERT (Mtype.array.empty () || Mtype.array.size () == 1);

        auto& structFields = f->structFields;

        parentFields.push_back (std::move (f));

        IterateTypeTree (compiler, type.member_types[i], structFields, depth + 1);
    }
}


struct SpirvParser::Impl {
    spirv_cross::Compiler compiler;

    Impl (const std::vector<uint32_t>& binary)
        : compiler (binary)
    {
    }
};


SpirvParser::SpirvParser (const std::vector<uint32_t>& binary)
    : impl { std::make_unique<Impl> (binary) }
{
}


SpirvParser::~SpirvParser () = default;


static std::vector<std::shared_ptr<UBO>> GetBufferObjectsFromBinary (SpirvParser& compiler_, const std::function<const spirv_cross::SmallVector<spirv_cross::Resource>&(const spirv_cross::ShaderResources&)> bufferResourceSelector)
{
    spirv_cross::Compiler& compiler = compiler_.impl->compiler;

    const spirv_cross::ShaderResources resources = compiler.get_shader_resources ();

    std::vector<std::shared_ptr<UBO>> ubos;

    for (auto& resource : bufferResourceSelector (resources)) {
        AllDecorations decorations (compiler, resource.id);
        auto           resType   = compiler.get_type (resource.type_id);
        const uint32_t arraySize = !resType.array.empty () ? resType.array[0] : 1;

        // using arrays on ubos will create seperate bindings,
        // eg. array of 4 on binding 2 will create 4 different bindings: 2, 3, 4, 5
        GVK_ASSERT (arraySize == 1);

        std::shared_ptr<UBO> root = std::make_unique<UBO> ();
        root->name                = resource.name;
        root->binding             = *decorations.Binding;
        root->descriptorSet       = *decorations.DescriptorSet;

        IterateTypeTree (compiler, resource.base_type_id, root->fields);

        const size_t declaredStructSize = compiler.get_declared_struct_size (compiler.get_type (resource.base_type_id));
        const size_t fullSize           = root->HasFixedSize () ? root->GetFullSize () : 0;

        root->hasDeclaredStructSize = declaredStructSize != 0;

        GVK_ASSERT (!root->hasDeclaredStructSize || declaredStructSize == fullSize);

        ubos.push_back (root);
    }

    std::sort (ubos.begin (), ubos.end (), [] (const std::shared_ptr<UBO>& first, const std::shared_ptr<UBO>& second) {
        return first->binding < second->binding;
    });

    return ubos;
}


std::vector<std::shared_ptr<UBO>> GetStorageBuffersFromBinary (SpirvParser& compiler_)
{
    return GetBufferObjectsFromBinary (compiler_, [] (const spirv_cross::ShaderResources& shaderResources) -> const spirv_cross::SmallVector<spirv_cross::Resource>& {
        return shaderResources.storage_buffers;
    });
}


std::vector<std::shared_ptr<UBO>> GetUBOsFromBinary (SpirvParser& compiler_)
{
    return GetBufferObjectsFromBinary (compiler_, [] (const spirv_cross::ShaderResources& shaderResources) -> const spirv_cross::SmallVector<spirv_cross::Resource>& {
        return shaderResources.uniform_buffers;
    });
}


std::vector<Output> GetOutputsFromBinary (SpirvParser& compiler_)
{
    spirv_cross::Compiler& compiler = compiler_.impl->compiler;

    const spirv_cross::ShaderResources resources = compiler.get_shader_resources ();

    std::vector<Output> result;

    for (auto& resource : resources.stage_outputs) {
        AllDecorations decorations (compiler, resource.id);
        auto           type = compiler.get_type (resource.type_id);

        Output output;

        output.name      = resource.name;
        output.location  = *decorations.Location;
        output.type      = BaseTypeNMToSRFieldType (type.basetype, type.vecsize, type.columns);
        output.arraySize = !type.array.empty () ? type.array[0] : 1;

        result.push_back (output);
    }

    std::sort (result.begin (), result.end (), [] (const Output& first, const Output& second) {
        return first.location < second.location;
    });

    return result;
}


VkFormat FieldTypeToVkFormat (FieldType fieldType)
{
    switch (fieldType) {
        case FieldType::Bool:
        case FieldType::Uint: return VK_FORMAT_R32_UINT;
        case FieldType::Int: return VK_FORMAT_R32_SINT;
        case FieldType::Float: return VK_FORMAT_R32_SFLOAT;
        case FieldType::Double: return VK_FORMAT_R64_SFLOAT;

        case FieldType::Uvec2:
        case FieldType::Bvec2: return VK_FORMAT_R32G32_UINT;
        case FieldType::Mat2x2:
        case FieldType::Mat3x2:
        case FieldType::Mat4x2:
        case FieldType::Vec2: return VK_FORMAT_R32G32_SFLOAT;
        case FieldType::Dmat2x2:
        case FieldType::Dmat3x2:
        case FieldType::Dmat4x2:
        case FieldType::Dvec2: return VK_FORMAT_R64G64_SFLOAT;

        case FieldType::Uvec3:
        case FieldType::Bvec3: return VK_FORMAT_R32G32B32_UINT;
        case FieldType::Mat2x3:
        case FieldType::Mat3x3:
        case FieldType::Mat4x3:
        case FieldType::Vec3: return VK_FORMAT_R32G32B32_SFLOAT;
        case FieldType::Dmat2x3:
        case FieldType::Dmat3x3:
        case FieldType::Dmat4x3:
        case FieldType::Dvec3: return VK_FORMAT_R64G64B64_SFLOAT;

        case FieldType::Uvec4:
        case FieldType::Bvec4: return VK_FORMAT_R32G32B32A32_UINT;
        case FieldType::Mat2x4:
        case FieldType::Mat3x4:
        case FieldType::Mat4x4:
        case FieldType::Vec4: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case FieldType::Dmat2x4:
        case FieldType::Dmat3x4:
        case FieldType::Dmat4x4:
        case FieldType::Dvec4: return VK_FORMAT_R64G64B64A64_SFLOAT;

        default:
            GVK_BREAK_STR ("no");
            throw std::runtime_error ("unable to convert FieldType to VkFormat");
    }
}


std::vector<SubpassInput> GetSubpassInputsFromBinary (SpirvParser& compiler_)
{
    spirv_cross::Compiler& compiler = compiler_.impl->compiler;

    const spirv_cross::ShaderResources resources = compiler.get_shader_resources ();

    std::vector<SubpassInput> result;

    for (auto& resource : resources.subpass_inputs) {
        AllDecorations decorations (compiler, resource.id);
        auto           type = compiler.get_type (resource.type_id);

        SubpassInput inp;

        inp.name         = resource.name;
        inp.binding      = *decorations.Binding;
        inp.subpassIndex = *decorations.InputAttachmentIndex;
        inp.type         = BaseTypeNMToSRFieldType (type.basetype, type.vecsize, type.columns);
        inp.arraySize    = !type.array.empty () ? type.array[0] : 1;

        result.push_back (inp);
    }

    std::sort (result.begin (), result.end (), [] (const SubpassInput& first, const SubpassInput& second) {
        return first.binding < second.binding;
    });

    return result;
}


std::vector<Input> GetInputsFromBinary (SpirvParser& compiler_)
{
    spirv_cross::Compiler& compiler = compiler_.impl->compiler;

    const spirv_cross::ShaderResources resources = compiler.get_shader_resources ();

    std::vector<Input> result;

    for (auto& resource : resources.stage_inputs) {
        AllDecorations decorations (compiler, resource.id);
        auto           type = compiler.get_type (resource.type_id);

        Input inp;

        inp.name        = resource.name;
        inp.location    = *decorations.Location;
        inp.type        = BaseTypeNMToSRFieldType (type.basetype, type.vecsize, type.columns);
        inp.arraySize   = !type.array.empty () ? type.array[0] : 1;
        inp.sizeInBytes = BaseTypeNMToByteSize (type.basetype, type.vecsize, type.columns);

        result.push_back (inp);
    }

    std::sort (result.begin (), result.end (), [] (const Input& first, const Input& second) {
        return first.location < second.location;
    });

    return result;
}


static Sampler::Type SpvDimToSamplerType (spv::Dim dim)
{
    switch (dim) {
        case spv::Dim::Dim1D: return Sampler::Type::Sampler1D;
        case spv::Dim::Dim2D: return Sampler::Type::Sampler2D;
        case spv::Dim::Dim3D: return Sampler::Type::Sampler3D;
        case spv::Dim::DimCube: return Sampler::Type::SamplerCube;

        default:
            GVK_BREAK ();
            throw std::runtime_error ("not supported type");
    }
}


std::vector<Sampler> GetSamplersFromBinary (SpirvParser& compiler_)
{
    spirv_cross::Compiler& compiler = compiler_.impl->compiler;

    const spirv_cross::ShaderResources resources = compiler.get_shader_resources ();

    std::vector<Sampler> result;

    for (auto& resource : resources.sampled_images) {
        AllDecorations decorations (compiler, resource.id);
        auto           type = compiler.get_type (resource.type_id);

        Sampler sampler;
        sampler.name          = resource.name;
        sampler.binding       = *decorations.Binding;
        sampler.descriptorSet = *decorations.DescriptorSet;
        sampler.type          = SpvDimToSamplerType (type.image.dim);
        sampler.arraySize     = !type.array.empty () ? type.array[0] : 1;

        GVK_ASSERT (type.array.empty () || type.array.size () == 1);

        result.push_back (sampler);
    }

    std::sort (result.begin (), result.end (), [] (const Sampler& first, const Sampler& second) {
        return first.binding < second.binding;
    });

    return result;
}


#define ENUM_TO_STRING_CASE(enumname, type) \
    case enumname::type:                    \
        return #type;
#define ENUM_TO_STRING_DEFAULT(enumname) \
    default: GVK_BREAK (); return #enumname "::[unknown]";


std::string FieldTypeToString (FieldType fieldType)
{
    switch (fieldType) {
        ENUM_TO_STRING_CASE (FieldType, Bool);
        ENUM_TO_STRING_CASE (FieldType, Int);
        ENUM_TO_STRING_CASE (FieldType, Uint);
        ENUM_TO_STRING_CASE (FieldType, Float);
        ENUM_TO_STRING_CASE (FieldType, Double);

        ENUM_TO_STRING_CASE (FieldType, Bvec2);
        ENUM_TO_STRING_CASE (FieldType, Ivec2);
        ENUM_TO_STRING_CASE (FieldType, Uvec2);
        ENUM_TO_STRING_CASE (FieldType, Vec2);
        ENUM_TO_STRING_CASE (FieldType, Dvec2);

        ENUM_TO_STRING_CASE (FieldType, Bvec3);
        ENUM_TO_STRING_CASE (FieldType, Ivec3);
        ENUM_TO_STRING_CASE (FieldType, Uvec3);
        ENUM_TO_STRING_CASE (FieldType, Vec3);
        ENUM_TO_STRING_CASE (FieldType, Dvec3);

        ENUM_TO_STRING_CASE (FieldType, Bvec4);
        ENUM_TO_STRING_CASE (FieldType, Ivec4);
        ENUM_TO_STRING_CASE (FieldType, Uvec4);
        ENUM_TO_STRING_CASE (FieldType, Vec4);
        ENUM_TO_STRING_CASE (FieldType, Dvec4);

        ENUM_TO_STRING_CASE (FieldType, Mat2x2);
        ENUM_TO_STRING_CASE (FieldType, Mat2x3);
        ENUM_TO_STRING_CASE (FieldType, Mat2x4);
        ENUM_TO_STRING_CASE (FieldType, Mat3x2);
        ENUM_TO_STRING_CASE (FieldType, Mat3x3);
        ENUM_TO_STRING_CASE (FieldType, Mat3x4);
        ENUM_TO_STRING_CASE (FieldType, Mat4x2);
        ENUM_TO_STRING_CASE (FieldType, Mat4x3);
        ENUM_TO_STRING_CASE (FieldType, Mat4x4);

        ENUM_TO_STRING_CASE (FieldType, Dmat2x2);
        ENUM_TO_STRING_CASE (FieldType, Dmat2x3);
        ENUM_TO_STRING_CASE (FieldType, Dmat2x4);
        ENUM_TO_STRING_CASE (FieldType, Dmat3x2);
        ENUM_TO_STRING_CASE (FieldType, Dmat3x3);
        ENUM_TO_STRING_CASE (FieldType, Dmat3x4);
        ENUM_TO_STRING_CASE (FieldType, Dmat4x2);
        ENUM_TO_STRING_CASE (FieldType, Dmat4x3);
        ENUM_TO_STRING_CASE (FieldType, Dmat4x4);

        ENUM_TO_STRING_CASE (FieldType, Struct);
        ENUM_TO_STRING_CASE (FieldType, Unknown);

        ENUM_TO_STRING_DEFAULT (FieldType);
    }
}

} // namespace SR
