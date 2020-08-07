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
    DecorName (GetDecor (compiler, resId, spv::Decoration##DecorName##))

#define MDEC(DecorName) \
    DecorName (GetMemberDecor (compiler, resId, spv::Decoration##DecorName##, memberIdx))

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

static SR::UBO::FieldType BaseTypeNMToSRFieldType (spirv_cross::SPIRType::BaseType b, uint32_t vecSize, uint32_t columns)
{
    using BaseType = spirv_cross::SPIRType::BaseType;

    switch (b) {
        case BaseType::Float:
            switch (columns) {
                case 1:
                    switch (vecSize) {
                        case 1: return SR::UBO::FieldType::Float;
                        case 2: return SR::UBO::FieldType::Vec2;
                        case 3: return SR::UBO::FieldType::Vec3;
                        case 4: return SR::UBO::FieldType::Vec4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 2:
                    switch (vecSize) {
                        case 2: return SR::UBO::FieldType::Mat2x2;
                        case 3: return SR::UBO::FieldType::Mat2x3;
                        case 4: return SR::UBO::FieldType::Mat2x4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 3:
                    switch (vecSize) {
                        case 2: return SR::UBO::FieldType::Mat3x2;
                        case 3: return SR::UBO::FieldType::Mat3x3;
                        case 4: return SR::UBO::FieldType::Mat3x4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 4:
                    switch (vecSize) {
                        case 2: return SR::UBO::FieldType::Mat4x2;
                        case 3: return SR::UBO::FieldType::Mat4x3;
                        case 4: return SR::UBO::FieldType::Mat4x4;
                        default: throw std::runtime_error ("wtf");
                    }
            }

        case BaseType::Double:
            switch (columns) {
                case 1:
                    switch (vecSize) {
                        case 1: return SR::UBO::FieldType::Double;
                        case 2: return SR::UBO::FieldType::Dvec2;
                        case 3: return SR::UBO::FieldType::Dvec3;
                        case 4: return SR::UBO::FieldType::Dvec4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 2:
                    switch (vecSize) {
                        case 2: return SR::UBO::FieldType::Dmat2x2;
                        case 3: return SR::UBO::FieldType::Dmat2x3;
                        case 4: return SR::UBO::FieldType::Dmat2x4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 3:
                    switch (vecSize) {
                        case 2: return SR::UBO::FieldType::Dmat3x2;
                        case 3: return SR::UBO::FieldType::Dmat3x3;
                        case 4: return SR::UBO::FieldType::Dmat3x4;
                        default: throw std::runtime_error ("wtf");
                    }
                case 4:
                    switch (vecSize) {
                        case 2: return SR::UBO::FieldType::Dmat4x2;
                        case 3: return SR::UBO::FieldType::Dmat4x3;
                        case 4: return SR::UBO::FieldType::Dmat4x4;
                        default: throw std::runtime_error ("wtf");
                    }
            }

        case BaseType::Boolean:
            switch (vecSize) {
                case 1: return SR::UBO::FieldType::Bool;
                case 2: return SR::UBO::FieldType::Bvec2;
                case 3: return SR::UBO::FieldType::Bvec3;
                case 4: return SR::UBO::FieldType::Bvec4;
                default: throw std::runtime_error ("wtf");
            }

        case BaseType::Int:
            switch (vecSize) {
                case 1: return SR::UBO::FieldType::Int;
                case 2: return SR::UBO::FieldType::Ivec2;
                case 3: return SR::UBO::FieldType::Ivec3;
                case 4: return SR::UBO::FieldType::Ivec4;
                default: throw std::runtime_error ("wtf");
            }

        case BaseType::UInt:
            switch (vecSize) {
                case 1: return SR::UBO::FieldType::Uint;
                case 2: return SR::UBO::FieldType::Uvec2;
                case 3: return SR::UBO::FieldType::Uvec3;
                case 4: return SR::UBO::FieldType::Uvec4;
                default: throw std::runtime_error ("wtf");
            }

        case BaseType::Struct:
            return SR::UBO::FieldType::Struct;

        default:
            ASSERT (false);
            return SR::UBO::FieldType::Unknown;
    }
}


static void IterateTypeTree (spirv_cross::Compiler& compiler, spirv_cross::TypeID typeId, std::vector<SR::UBO::FieldP>& parentFields, const uint32_t depth = 0)
{
    const spirv_cross::SPIRType& type = compiler.get_type (typeId);

    const uint32_t memberCount = type.member_types.size ();

    for (uint32_t i = 0; i < memberCount; ++i) {
        AllDecorations               typeMemDecorA (compiler, type.member_types[i]);
        AllDecorations               typeMemDecor (compiler, type.self, i);
        const spirv_cross::SPIRType& Mtype = compiler.get_type (type.member_types[i]);

        SR::UBO::FieldP f = SR::UBO::Field::Create ();
        f->name           = typeMemDecor.name;
        f->offset         = *typeMemDecor.Offset;
        f->arrayStride    = typeMemDecorA.ArrayStride ? *typeMemDecorA.ArrayStride : 0;
        f->arraySize      = !Mtype.array.empty () ? Mtype.array[0] : 0;
        f->size           = (Mtype.width * Mtype.vecsize * Mtype.columns) / 8;
        f->type           = BaseTypeNMToSRFieldType (Mtype.basetype, Mtype.vecsize, Mtype.columns);

        parentFields.push_back (f);

        IterateTypeTree (compiler, type.member_types[i], f->structFields, depth + 1);
    }
}

std::vector<UBO> GetUBOsFromBinary (const std::vector<uint32_t>& binary)
{
    spirv_cross::Compiler compiler (binary);

    const spirv_cross::ShaderResources resources = compiler.get_shader_resources ();

    std::vector<SR::UBO> ubos;

    for (auto& resource : resources.uniform_buffers) {
        AllDecorations decorations (compiler, resource.id);

        SR::UBO root;
        root.name          = resource.name;
        root.binding       = *decorations.Binding;
        root.descriptorSet = *decorations.DescriptorSet;

        IterateTypeTree (compiler, resource.base_type_id, root.fields);

        ASSERT (compiler.get_declared_struct_size (compiler.get_type (resource.base_type_id)) == root.GetFullSize ());
        
        ubos.push_back (root);
    }

    return ubos;
}

}; // namespace SR