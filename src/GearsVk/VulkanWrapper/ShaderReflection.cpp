#include "ShaderReflection.hpp"

#include "spirv_cross.hpp"

#include <optional>
#include <sstream>

namespace SR {

struct AllDecorations {
    std::string             name;
    std::optional<uint32_t> RelaxedPrecision;
    std::optional<uint32_t> SpecId;
    std::optional<uint32_t> Block;
    std::optional<uint32_t> BufferBlock;
    std::optional<uint32_t> RowMajor;
    std::optional<uint32_t> ColMajor;
    std::optional<uint32_t> ArrayStride;
    std::optional<uint32_t> MatrixStride;
    std::optional<uint32_t> GLSLShared;
    std::optional<uint32_t> GLSLPacked;
    std::optional<uint32_t> CPacked;
    std::optional<uint32_t> BuiltIn;
    std::optional<uint32_t> NoPerspective;
    std::optional<uint32_t> Flat;
    std::optional<uint32_t> Patch;
    std::optional<uint32_t> Centroid;
    std::optional<uint32_t> Sample;
    std::optional<uint32_t> Invariant;
    std::optional<uint32_t> Restrict;
    std::optional<uint32_t> Aliased;
    std::optional<uint32_t> Volatile;
    std::optional<uint32_t> Constant;
    std::optional<uint32_t> Coherent;
    std::optional<uint32_t> NonWritable;
    std::optional<uint32_t> NonReadable;
    std::optional<uint32_t> Uniform;
    std::optional<uint32_t> UniformId;
    std::optional<uint32_t> SaturatedConversion;
    std::optional<uint32_t> Stream;
    std::optional<uint32_t> Location;
    std::optional<uint32_t> Component;
    std::optional<uint32_t> Index;
    std::optional<uint32_t> Binding;
    std::optional<uint32_t> DescriptorSet;
    std::optional<uint32_t> Offset;
    std::optional<uint32_t> XfbBuffer;
    std::optional<uint32_t> XfbStride;
    std::optional<uint32_t> FuncParamAttr;
    std::optional<uint32_t> FPRoundingMode;
    std::optional<uint32_t> FPFastMathMode;
    std::optional<uint32_t> LinkageAttributes;
    std::optional<uint32_t> NoContraction;
    std::optional<uint32_t> InputAttachmentIndex;
    std::optional<uint32_t> Alignment;
    std::optional<uint32_t> MaxByteOffset;
    std::optional<uint32_t> AlignmentId;
    std::optional<uint32_t> MaxByteOffsetId;
    std::optional<uint32_t> NoSignedWrap;
    std::optional<uint32_t> NoUnsignedWrap;
    std::optional<uint32_t> ExplicitInterpAMD;
    std::optional<uint32_t> OverrideCoverageNV;
    std::optional<uint32_t> PassthroughNV;
    std::optional<uint32_t> ViewportRelativeNV;
    std::optional<uint32_t> SecondaryViewportRelativeNV;
    std::optional<uint32_t> PerPrimitiveNV;
    std::optional<uint32_t> PerViewNV;
    std::optional<uint32_t> PerTaskNV;
    std::optional<uint32_t> PerVertexNV;
    std::optional<uint32_t> NonUniform;
    std::optional<uint32_t> NonUniformEXT;
    std::optional<uint32_t> RestrictPointer;
    std::optional<uint32_t> RestrictPointerEXT;
    std::optional<uint32_t> AliasedPointer;
    std::optional<uint32_t> AliasedPointerEXT;
    std::optional<uint32_t> CounterBuffer;
    std::optional<uint32_t> HlslCounterBufferGOOGLE;
    std::optional<uint32_t> HlslSemanticGOOGLE;
    std::optional<uint32_t> UserSemantic;
    std::optional<uint32_t> UserTypeGOOGLE;

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

#define DEC(DecorName) \
    DecorName (GetDecor (compiler, resId, spv::Decoration##DecorName))

#define MDEC(DecorName) \
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
        , DEC (RelaxedPrecision)
        , DEC (SpecId)
        , DEC (Block)
        , DEC (BufferBlock)
        , DEC (RowMajor)
        , DEC (ColMajor)
        , DEC (ArrayStride)
        , DEC (MatrixStride)
        , DEC (GLSLShared)
        , DEC (GLSLPacked)
        , DEC (CPacked)
        , DEC (BuiltIn)
        , DEC (NoPerspective)
        , DEC (Flat)
        , DEC (Patch)
        , DEC (Centroid)
        , DEC (Sample)
        , DEC (Invariant)
        , DEC (Restrict)
        , DEC (Aliased)
        , DEC (Volatile)
        , DEC (Constant)
        , DEC (Coherent)
        , DEC (NonWritable)
        , DEC (NonReadable)
        , DEC (Uniform)
        , DEC (UniformId)
        , DEC (SaturatedConversion)
        , DEC (Stream)
        , DEC (Location)
        , DEC (Component)
        , DEC (Index)
        , DEC (Binding)
        , DEC (DescriptorSet)
        , DEC (Offset)
        , DEC (XfbBuffer)
        , DEC (XfbStride)
        , DEC (FuncParamAttr)
        , DEC (FPRoundingMode)
        , DEC (FPFastMathMode)
        , DEC (LinkageAttributes)
        , DEC (NoContraction)
        , DEC (InputAttachmentIndex)
        , DEC (Alignment)
        , DEC (MaxByteOffset)
        , DEC (AlignmentId)
        , DEC (MaxByteOffsetId)
        , DEC (NoSignedWrap)
        , DEC (NoUnsignedWrap)
        , DEC (ExplicitInterpAMD)
        , DEC (OverrideCoverageNV)
        , DEC (PassthroughNV)
        , DEC (ViewportRelativeNV)
        , DEC (SecondaryViewportRelativeNV)
        , DEC (PerPrimitiveNV)
        , DEC (PerViewNV)
        , DEC (PerTaskNV)
        , DEC (PerVertexNV)
        , DEC (NonUniform)
        , DEC (NonUniformEXT)
        , DEC (RestrictPointer)
        , DEC (RestrictPointerEXT)
        , DEC (AliasedPointer)
        , DEC (AliasedPointerEXT)
        , DEC (CounterBuffer)
        , DEC (HlslCounterBufferGOOGLE)
        , DEC (HlslSemanticGOOGLE)
        , DEC (UserSemantic)
        , DEC (UserTypeGOOGLE)
    {
    }

    AllDecorations (const spirv_cross::Compiler& compiler, spirv_cross::ID resId, uint32_t memberIdx)
        : name (compiler.get_member_name (resId, memberIdx))
        , MDEC (RelaxedPrecision)
        , MDEC (SpecId)
        , MDEC (Block)
        , MDEC (BufferBlock)
        , MDEC (RowMajor)
        , MDEC (ColMajor)
        , MDEC (ArrayStride)
        , MDEC (MatrixStride)
        , MDEC (GLSLShared)
        , MDEC (GLSLPacked)
        , MDEC (CPacked)
        , MDEC (BuiltIn)
        , MDEC (NoPerspective)
        , MDEC (Flat)
        , MDEC (Patch)
        , MDEC (Centroid)
        , MDEC (Sample)
        , MDEC (Invariant)
        , MDEC (Restrict)
        , MDEC (Aliased)
        , MDEC (Volatile)
        , MDEC (Constant)
        , MDEC (Coherent)
        , MDEC (NonWritable)
        , MDEC (NonReadable)
        , MDEC (Uniform)
        , MDEC (UniformId)
        , MDEC (SaturatedConversion)
        , MDEC (Stream)
        , MDEC (Location)
        , MDEC (Component)
        , MDEC (Index)
        , MDEC (Binding)
        , MDEC (DescriptorSet)
        , MDEC (Offset)
        , MDEC (XfbBuffer)
        , MDEC (XfbStride)
        , MDEC (FuncParamAttr)
        , MDEC (FPRoundingMode)
        , MDEC (FPFastMathMode)
        , MDEC (LinkageAttributes)
        , MDEC (NoContraction)
        , MDEC (InputAttachmentIndex)
        , MDEC (Alignment)
        , MDEC (MaxByteOffset)
        , MDEC (AlignmentId)
        , MDEC (MaxByteOffsetId)
        , MDEC (NoSignedWrap)
        , MDEC (NoUnsignedWrap)
        , MDEC (ExplicitInterpAMD)
        , MDEC (OverrideCoverageNV)
        , MDEC (PassthroughNV)
        , MDEC (ViewportRelativeNV)
        , MDEC (SecondaryViewportRelativeNV)
        , MDEC (PerPrimitiveNV)
        , MDEC (PerViewNV)
        , MDEC (PerTaskNV)
        , MDEC (PerVertexNV)
        , MDEC (NonUniform)
        , MDEC (NonUniformEXT)
        , MDEC (RestrictPointer)
        , MDEC (RestrictPointerEXT)
        , MDEC (AliasedPointer)
        , MDEC (AliasedPointerEXT)
        , MDEC (CounterBuffer)
        , MDEC (HlslCounterBufferGOOGLE)
        , MDEC (HlslSemanticGOOGLE)
        , MDEC (UserSemantic)
        , MDEC (UserTypeGOOGLE)
    {
    }

    std::string ToString () const
    {
        std::stringstream ss;

        if (!name.empty ()) {
            ss << "\"" << name << "\": ";
        }

#define TOSTR(val)                           \
    if (val) {                               \
        ss << #val << " = " << *val << ", "; \
    }

        TOSTR (RelaxedPrecision)
        TOSTR (SpecId)
        TOSTR (Block)
        TOSTR (BufferBlock)
        TOSTR (RowMajor)
        TOSTR (ColMajor)
        TOSTR (ArrayStride)
        TOSTR (MatrixStride)
        TOSTR (GLSLShared)
        TOSTR (GLSLPacked)
        TOSTR (CPacked)
        TOSTR (BuiltIn)
        TOSTR (NoPerspective)
        TOSTR (Flat)
        TOSTR (Patch)
        TOSTR (Centroid)
        TOSTR (Sample)
        TOSTR (Invariant)
        TOSTR (Restrict)
        TOSTR (Aliased)
        TOSTR (Volatile)
        TOSTR (Constant)
        TOSTR (Coherent)
        TOSTR (NonWritable)
        TOSTR (NonReadable)
        TOSTR (Uniform)
        TOSTR (UniformId)
        TOSTR (SaturatedConversion)
        TOSTR (Stream)
        TOSTR (Location)
        TOSTR (Component)
        TOSTR (Index)
        TOSTR (Binding)
        TOSTR (DescriptorSet)
        TOSTR (Offset)
        TOSTR (XfbBuffer)
        TOSTR (XfbStride)
        TOSTR (FuncParamAttr)
        TOSTR (FPRoundingMode)
        TOSTR (FPFastMathMode)
        TOSTR (LinkageAttributes)
        TOSTR (NoContraction)
        TOSTR (InputAttachmentIndex)
        TOSTR (Alignment)
        TOSTR (MaxByteOffset)
        TOSTR (AlignmentId)
        TOSTR (MaxByteOffsetId)
        TOSTR (NoSignedWrap)
        TOSTR (NoUnsignedWrap)
        TOSTR (ExplicitInterpAMD)
        TOSTR (OverrideCoverageNV)
        TOSTR (PassthroughNV)
        TOSTR (ViewportRelativeNV)
        TOSTR (SecondaryViewportRelativeNV)
        TOSTR (PerPrimitiveNV)
        TOSTR (PerViewNV)
        TOSTR (PerTaskNV)
        TOSTR (PerVertexNV)
        TOSTR (NonUniform)
        TOSTR (NonUniformEXT)
        TOSTR (RestrictPointer)
        TOSTR (RestrictPointerEXT)
        TOSTR (AliasedPointer)
        TOSTR (AliasedPointerEXT)
        TOSTR (CounterBuffer)
        TOSTR (HlslCounterBufferGOOGLE)
        TOSTR (HlslSemanticGOOGLE)
        TOSTR (UserSemantic)
        TOSTR (UserTypeGOOGLE)

        return ss.str ();
    }
};

#define CASERETURN(type) \
    case type: return #type;

static std::string BaseTypeToString (spirv_cross::SPIRType::BaseType b)
{
    using BaseType = spirv_cross::SPIRType::BaseType;

    switch (b) {
        CASERETURN (BaseType::Unknown)
        CASERETURN (BaseType::Void)
        CASERETURN (BaseType::Boolean)
        CASERETURN (BaseType::SByte)
        CASERETURN (BaseType::UByte)
        CASERETURN (BaseType::Short)
        CASERETURN (BaseType::UShort)
        CASERETURN (BaseType::Int)
        CASERETURN (BaseType::UInt)
        CASERETURN (BaseType::Int64)
        CASERETURN (BaseType::UInt64)
        CASERETURN (BaseType::AtomicCounter)
        CASERETURN (BaseType::Half)
        CASERETURN (BaseType::Float)
        CASERETURN (BaseType::Double)
        CASERETURN (BaseType::Struct)
        CASERETURN (BaseType::Image)
        CASERETURN (BaseType::SampledImage)
        CASERETURN (BaseType::Sampler)
        CASERETURN (BaseType::AccelerationStructure)
        CASERETURN (BaseType::RayQuery)
        CASERETURN (BaseType::ControlPointArray)
        CASERETURN (BaseType::Char)
        default: return "";
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
    }
}


static uint32_t BaseTypeNMToByteSize (spirv_cross::SPIRType::BaseType b, uint32_t vecSize, uint32_t columns)
{
    return BaseTypeToByteSize (b) * vecSize * columns;
}


static SR::FieldType BaseTypeNMToSRFieldType (spirv_cross::SPIRType::BaseType b, uint32_t vecSize, uint32_t columns)
{
    using BaseType = spirv_cross::SPIRType::BaseType;

    switch (b) {
        case BaseType::Float:
            switch (columns) {
                case 1:
                    switch (vecSize) {
                        case 1: return SR::FieldType::Float;
                        case 2: return SR::FieldType::Vec2;
                        case 3: return SR::FieldType::Vec3;
                        case 4: return SR::FieldType::Vec4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 2:
                    switch (vecSize) {
                        case 2: return SR::FieldType::Mat2x2;
                        case 3: return SR::FieldType::Mat2x3;
                        case 4: return SR::FieldType::Mat2x4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 3:
                    switch (vecSize) {
                        case 2: return SR::FieldType::Mat3x2;
                        case 3: return SR::FieldType::Mat3x3;
                        case 4: return SR::FieldType::Mat3x4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 4:
                    switch (vecSize) {
                        case 2: return SR::FieldType::Mat4x2;
                        case 3: return SR::FieldType::Mat4x3;
                        case 4: return SR::FieldType::Mat4x4;
                        default: throw std::runtime_error ("wtf");
                    }
            }

        case BaseType::Double:
            switch (columns) {
                case 1:
                    switch (vecSize) {
                        case 1: return SR::FieldType::Double;
                        case 2: return SR::FieldType::Dvec2;
                        case 3: return SR::FieldType::Dvec3;
                        case 4: return SR::FieldType::Dvec4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 2:
                    switch (vecSize) {
                        case 2: return SR::FieldType::Dmat2x2;
                        case 3: return SR::FieldType::Dmat2x3;
                        case 4: return SR::FieldType::Dmat2x4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 3:
                    switch (vecSize) {
                        case 2: return SR::FieldType::Dmat3x2;
                        case 3: return SR::FieldType::Dmat3x3;
                        case 4: return SR::FieldType::Dmat3x4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 4:
                    switch (vecSize) {
                        case 2: return SR::FieldType::Dmat4x2;
                        case 3: return SR::FieldType::Dmat4x3;
                        case 4: return SR::FieldType::Dmat4x4;
                        default: throw std::runtime_error ("wtf");
                    }
            }

        case BaseType::Boolean:
            switch (vecSize) {
                case 1: return SR::FieldType::Bool;
                case 2: return SR::FieldType::Bvec2;
                case 3: return SR::FieldType::Bvec3;
                case 4: return SR::FieldType::Bvec4;
                default: throw std::runtime_error ("wtf");
            }

        case BaseType::Int:
            switch (vecSize) {
                case 1: return SR::FieldType::Int;
                case 2: return SR::FieldType::Ivec2;
                case 3: return SR::FieldType::Ivec3;
                case 4: return SR::FieldType::Ivec4;
                default: throw std::runtime_error ("wtf");
            }

        case BaseType::UInt:
            switch (vecSize) {
                case 1: return SR::FieldType::Uint;
                case 2: return SR::FieldType::Uvec2;
                case 3: return SR::FieldType::Uvec3;
                case 4: return SR::FieldType::Uvec4;
                default: throw std::runtime_error ("wtf");
            }

        case BaseType::Struct:
            return SR::FieldType::Struct;

        default:
            GVK_ASSERT (false);
            return SR::FieldType::Unknown;
    }
}


static void IterateTypeTree (spirv_cross::Compiler& compiler, spirv_cross::TypeID typeId, std::vector<SR::FieldP>& parentFields, const uint32_t depth = 0)
{
    const spirv_cross::SPIRType& type = compiler.get_type (typeId);

    const uint32_t memberCount = type.member_types.size ();

    for (uint32_t i = 0; i < memberCount; ++i) {
        AllDecorations               typeMemDecorA (compiler, type.member_types[i]);
        AllDecorations               typeMemDecor (compiler, type.self, i);
        const spirv_cross::SPIRType& Mtype = compiler.get_type (type.member_types[i]);

        SR::FieldP f   = SR::Field::Create ();
        f->name        = typeMemDecor.name;
        f->offset      = *typeMemDecor.Offset;
        f->arrayStride = typeMemDecorA.ArrayStride ? *typeMemDecorA.ArrayStride : 0;
        f->arraySize   = !Mtype.array.empty () ? Mtype.array[0] : 0;
        f->size        = (Mtype.width * Mtype.vecsize * Mtype.columns) / 8;
        f->type        = BaseTypeNMToSRFieldType (Mtype.basetype, Mtype.vecsize, Mtype.columns);

        GVK_ASSERT (Mtype.array.empty () || Mtype.array.size () == 1);

        parentFields.push_back (f);

        IterateTypeTree (compiler, type.member_types[i], f->structFields, depth + 1);
    }
}


std::vector<UBOP> GetUBOsFromBinary (const std::vector<uint32_t>& binary)
{
    spirv_cross::Compiler compiler (binary);

    const spirv_cross::ShaderResources resources = compiler.get_shader_resources ();

    std::vector<SR::UBOP> ubos;

    for (auto& resource : resources.uniform_buffers) {
        AllDecorations decorations (compiler, resource.id);
        auto           resType   = compiler.get_type (resource.type_id);
        const uint32_t arraySize = !resType.array.empty () ? resType.array[0] : 0;

        // using arrays on ubos will create seperate bindings,
        // eg. array of 4 on binding 2 will create 4 different bindings: 2, 3, 4, 5
        GVK_ASSERT (arraySize == 0);

        SR::UBOP root       = SR::UBO::Create ();
        root->name          = resource.name;
        root->binding       = *decorations.Binding;
        root->descriptorSet = *decorations.DescriptorSet;

        IterateTypeTree (compiler, resource.base_type_id, root->fields);

        GVK_ASSERT (compiler.get_declared_struct_size (compiler.get_type (resource.base_type_id)) == root->GetFullSize ());

        ubos.push_back (root);
    }

    std::sort (ubos.begin (), ubos.end (), [] (const SR::UBOP& first, const SR::UBOP& second) {
        return first->binding < second->binding;
    });

    return ubos;
}


std::vector<Output> GetOutputsFromBinary (const std::vector<uint32_t>& binary)
{
    spirv_cross::Compiler compiler (binary);

    const spirv_cross::ShaderResources resources = compiler.get_shader_resources ();

    std::vector<SR::Output> result;

    for (auto& resource : resources.stage_outputs) {
        AllDecorations decorations (compiler, resource.id);
        auto           type = compiler.get_type (resource.type_id);

        Output output;

        output.name      = resource.name;
        output.location  = *decorations.Location;
        output.type      = BaseTypeNMToSRFieldType (type.basetype, type.vecsize, type.columns);
        output.arraySize = !type.array.empty () ? type.array[0] : 0;

        result.push_back (output);
    }

    std::sort (result.begin (), result.end (), [] (const SR::Output& first, const SR::Output& second) {
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
            GVK_BREAK ("no");
            throw std::runtime_error ("unable to convert FieldType to VkFormat");
    }
}


std::vector<Input> GetInputsFromBinary (const std::vector<uint32_t>& binary)
{
    spirv_cross::Compiler compiler (binary);

    const spirv_cross::ShaderResources resources = compiler.get_shader_resources ();

    std::vector<SR::Input> result;

    for (auto& resource : resources.stage_inputs) {
        AllDecorations decorations (compiler, resource.id);
        auto           type = compiler.get_type (resource.type_id);

        Input inp;

        inp.name        = resource.name;
        inp.location    = *decorations.Location;
        inp.type        = BaseTypeNMToSRFieldType (type.basetype, type.vecsize, type.columns);
        inp.arraySize   = !type.array.empty () ? type.array[0] : 0;
        inp.sizeInBytes = BaseTypeNMToByteSize (type.basetype, type.vecsize, type.columns);

        result.push_back (inp);
    }

    std::sort (result.begin (), result.end (), [] (const SR::Input& first, const SR::Input& second) {
        return first.location < second.location;
    });

    return result;
}


static SR::Sampler::Type SpvDimToSamplerType (spv::Dim dim)
{
    switch (dim) {
        case spv::Dim::Dim1D: return SR::Sampler::Type::Sampler1D;
        case spv::Dim::Dim2D: return SR::Sampler::Type::Sampler2D;
        case spv::Dim::Dim3D: return SR::Sampler::Type::Sampler3D;
        case spv::Dim::DimCube: return SR::Sampler::Type::SamplerCube;

        default:
            GVK_ASSERT (false);
            throw std::runtime_error ("not supported type");
    }
}


std::vector<Sampler> GetSamplersFromBinary (const std::vector<uint32_t>& binary)
{
    spirv_cross::Compiler compiler (binary);

    const spirv_cross::ShaderResources resources = compiler.get_shader_resources ();

    std::vector<SR::Sampler> result;

    for (auto& resource : resources.sampled_images) {
        AllDecorations decorations (compiler, resource.id);
        auto           type = compiler.get_type (resource.type_id);

        SR::Sampler sampler;
        sampler.name          = resource.name;
        sampler.binding       = *decorations.Binding;
        sampler.descriptorSet = *decorations.DescriptorSet;
        sampler.type          = SpvDimToSamplerType (type.image.dim);
        sampler.arraySize     = !type.array.empty () ? type.array[0] : 0;

        GVK_ASSERT (type.array.empty () || type.array.size () == 1);

        result.push_back (sampler);
    }

    std::sort (result.begin (), result.end (), [] (const SR::Sampler& first, const SR::Sampler& second) {
        return first.binding < second.binding;
    });

    return result;
}


#define ENUM_TO_STRING_CASE(enumname, type) \
    case enumname::type:                    \
        return #type;
#define ENUM_TO_STRING_DEFAULT(enumname) \
    default: GVK_ASSERT (false); return #enumname "::[unknown]";


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

}; // namespace SR