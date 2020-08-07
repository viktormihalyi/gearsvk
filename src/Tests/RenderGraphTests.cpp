#include <vulkan/vulkan.h>

#include "FullscreenQuad.hpp"
#include "GraphRenderer.hpp"
#include "Ptr.hpp"
#include "RenderGraph.hpp"
#include "ShaderPipeline.hpp"
#include "Timer.hpp"
#include "UniformBlock.hpp"
#include "Utils.hpp"
#include "VulkanUtils.hpp"
#include "VulkanWrapper.hpp"

#include "glmlib.hpp"

#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include "GoogleTestEnvironment.hpp"
#include "SDF.hpp"


const std::filesystem::path ShadersFolder = PROJECT_ROOT / "src" / "Tests" / "shaders";


using namespace RG;

#include "spirv_cross.hpp"


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

std::string BaseTypeToString (spirv_cross::SPIRType::BaseType b)
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

SR::UBO::FieldType BaseTypeNMToSRFieldType (spirv_cross::SPIRType::BaseType b, uint32_t vecSize, uint32_t columns)
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

#include <algorithm>

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


class PublicIRCompiler : public spirv_cross::Compiler {
public:
    PublicIRCompiler (std::vector<uint32_t> ir)
        : spirv_cross::Compiler (ir)
    {
    }

    spirv_cross::ParsedIR& GetIR ()
    {
        return ir;
    }

    std::string GetVariableName (spirv_cross::ID id, uint32_t memberIndex) const
    {
        ASSERT (ir.meta.find (id) != ir.meta.end ());
        ASSERT (memberIndex < ir.meta.at (id).members.size ());
        return ir.meta.at (id).members[memberIndex].alias;
    }
};


TEST_F (HiddenWindowGoogleTestEnvironment, DISABLED_SpirvCroos)
{
    // Read SPIR-V from disk or similar.

    std::vector<uint32_t> spirv_binary  = ShaderModule::CreateFromGLSLFile (GetDevice (), PROJECT_ROOT / "src" / "VizHF" / "shaders" / "brain.frag")->GetBinary ();
    std::vector<uint32_t> spirv_binary2 = ShaderModule::CreateFromSPVFile (GetDevice (), ShaderModule::ShaderKind::Fragment, PROJECT_ROOT / "src" / "VizHF" / "shaders" / "a.spv")->GetBinary ();

    PublicIRCompiler glsl (spirv_binary2);

    for (auto [id, met] : glsl.GetIR ().meta) {
        if (met.decoration.matrix_stride > 0) {
            ASSERT (false);
        }
    }

    spirv_cross::ShaderResources resources = glsl.get_shader_resources ();

    for (auto& resource : resources.sampled_images) {
        unsigned set     = glsl.get_decoration (resource.id, spv::DecorationDescriptorSet);
        unsigned binding = glsl.get_decoration (resource.id, spv::DecorationBinding);
        printf ("Image %s at set = %u, binding = %u\n", resource.name.c_str (), set, binding);
    }

    for (auto& resource : resources.uniform_buffers) {
        AllDecorations decorations (glsl, resource.id);
        //IterateTypeTree (glsl, resource.base_type_id);
        //IterateTypeTree (glsl, resource.type_id);
    }
}


TEST_F (HiddenWindowGoogleTestEnvironment, Spirvrross2)
{
    auto sm = ShaderModule::CreateFromGLSLString (GetDevice (), ShaderModule::ShaderKind::Fragment, R"(#version 450

struct A {
    vec3 abc;
};

struct B {
    A bs[3];
};

struct C {
    B cs[3];
    float hehe;
};

layout (std140, binding = 2) uniform Quadrics {
    float dddddddddddd;
    vec3 dddddddddddd33;
    C quadrics[2];
    C theotherone[9];
};

layout (location = 0) out vec4 presented;

void main ()
{
    presented = vec4 (vec3 (1), dddddddddddd);
}

)");

    sm = ShaderModule::CreateFromSPVFile (GetDevice (), ShaderModule::ShaderKind::Fragment, PROJECT_ROOT / "src" / "Tests" / "shaders" / "a.spv");

    PublicIRCompiler compiler (sm->GetBinary ());

    for (auto [id, met] : compiler.GetIR ().meta) {
        if (met.decoration.matrix_stride > 0) {
            ASSERT (false);
        }
    }

    spirv_cross::ShaderResources resources = compiler.get_shader_resources ();

    std::vector<SR::UBO> ubos;

    for (auto& resource : resources.uniform_buffers) {
        AllDecorations decorations (compiler, resource.id);

        SR::UBO root;
        root.name          = resource.name;
        root.binding       = *decorations.Binding;
        root.descriptorSet = *decorations.DescriptorSet;

        IterateTypeTree (compiler, resource.base_type_id, root.fields);

        std::cout << compiler.get_declared_struct_size (compiler.get_type (resource.base_type_id)) << std::endl;

        std::cout << root.GetFullSize () << std::endl;
        ubos.push_back (root);
    }
}


TEST_F (HiddenWindowGoogleTestEnvironment, DISABLED_MSDFGEN)
{
    // Image2DTransferable glyphs (GetDevice (), GetGraphicsQueue (), GetCommandPool (), VK_FORMAT_R8G8B8A8_UINT, 16, 16, VK_IMAGE_USAGE_SAMPLED_BIT, 128);

    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    GraphSettings s (device, graphicsQueue, commandPool, 3, 512, 512);
    RenderGraph   graph (device, commandPool);

    WritableImageResource& red    = graph.CreateResource<WritableImageResource> ();
    ReadOnlyImageResource& glyphs = graph.CreateResource<ReadOnlyImageResource> (VK_FORMAT_R32_SFLOAT, 32, 32, 1, 512);

    auto sp = ShaderPipeline::Create (device);
    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )");

    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 textureCoords;
layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2DArray sampl;

void main () {
    outColor = vec4 (vec3 (texture (sampl, vec3 (textureCoords, 2)).r), 1.f);
}
    )");

    Operation& redFillOperation = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 6),
                                                                               std::move (sp)));

    graph.CreateOutputConnection (redFillOperation, 0, red);
    graph.CreateInputConnection<ImageInputBinding> (redFillOperation, 0, glyphs);

    graph.Compile (s);


    std::map<char, uint32_t> charToLayerMapping;
    std::vector<float>       asd;
    for (uint32_t i = 0; i < 255; ++i) {
        if (i == 32 || i == 160)
            continue;
        asd = GetGlyphSDF32x32x1 ("C:\\Windows\\Fonts\\arialbd.ttf", i);
        glyphs.CopyLayer (asd, i);
        charToLayerMapping[i] = i;
    }


    graph.Submit (0);
    graph.Submit (1);
    graph.Submit (2);

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    for (uint32_t i = 0; i < 255; ++i) {
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *glyphs.image->imageGPU->image, ReferenceImagesFolder / ("" + std::to_string (i) + "glyphA.png"), i).join ();
    }

    //SaveImageToFileAsync (device, graphicsQueue, commandPool, *glyphs.image->imageGPU->image, ReferenceImagesFolder / "glyphA.png", 0).join ();
    //SaveImageToFileAsync (device, graphicsQueue, commandPool, *glyphs.image->imageGPU->image, ReferenceImagesFolder / "glyphB.png", 1).join ();
    //SaveImageToFileAsync (device, graphicsQueue, commandPool, *glyphs.image->imageGPU->image, ReferenceImagesFolder / "glyphC.png", 2).join ();
    SaveImageToFileAsync (device, graphicsQueue, commandPool, *red.images[0]->image.image, ReferenceImagesFolder / "glyphAout.png").join ();
}

TEST_F (EmptyTestEnvironment, UniformBlockTest)
{
    using namespace ST;

    {
        ShaderStruct b ({
            {"f", ST::vec1},
            {"m", ST::mat4},
        });
        EXPECT_EQ (80, b.GetFullSize ());
        EXPECT_EQ (0, b.GetOffset ("f"));
        EXPECT_EQ (16, b.GetOffset ("m"));
    }

    {
        ShaderStruct b ({
            {"m", ST::mat4},
            {"f", ST::vec1},
        });
        EXPECT_EQ (68, b.GetFullSize ());
        EXPECT_EQ (64, b.GetOffset ("f"));
        EXPECT_EQ (0, b.GetOffset ("m"));
    }

    {
        ShaderStruct b ({
            {"f", ST::vec1},
            {"m", ST::mat4},
            {"f2", ST::vec1},
        });
        EXPECT_EQ (80, b.GetOffset ("f2"));
    }

    {
        ShaderStruct b ({
            {"asd", vec1},
            {"4635", mat4},
            {"fd", vec4},
            {"f4", vec3},
            {"865", vec4Array<3>},
            {"23", vec1Array<6>},
        });

        ASSERT (b.GetOffset ("4635") == 16);
    }

    EXPECT_TRUE (true);
}


TEST_F (HeadlessGoogleTestEnvironment, DISABLED_RenderGraphConnectionTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    RenderGraph graph (device, commandPool);

    Resource& depthBuffer    = graph.AddResource (WritableImageResource::Create ());
    Resource& depthBuffer2   = graph.AddResource (WritableImageResource::Create ());
    Resource& gbuffer1       = graph.AddResource (WritableImageResource::Create ());
    Resource& gbuffer2       = graph.AddResource (WritableImageResource::Create ());
    Resource& gbuffer3       = graph.AddResource (WritableImageResource::Create ());
    Resource& debugOutput    = graph.AddResource (WritableImageResource::Create ());
    Resource& lightingBuffer = graph.AddResource (WritableImageResource::Create ());
    Resource& finalTarget    = graph.AddResource (WritableImageResource::Create ());

    Operation& depthPass   = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& gbufferPass = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& debugView   = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& move        = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& lighting    = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& post        = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));
    Operation& present     = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3), ShaderPipeline::CreateShared (device)));

    // depthPass.AddOutput (0, depthBuffer);
    //
    // gbufferPass.AddInput (0, depthBuffer);
    // gbufferPass.AddOutput (0, depthBuffer2);
    // gbufferPass.AddOutput (1, gbuffer1);
    // gbufferPass.AddOutput (2, gbuffer2);
    // gbufferPass.AddOutput (3, gbuffer3);
    //
    // debugView.AddInput (0, gbuffer3);
    // debugView.AddOutput (0, debugOutput);
    //
    // lighting.AddInput (0, depthBuffer);
    // lighting.AddInput (1, gbuffer1);
    // lighting.AddInput (2, gbuffer2);
    // lighting.AddInput (3, gbuffer3);
    // lighting.AddOutput (0, lightingBuffer);
    //
    // post.AddInput (0, lightingBuffer);
    // post.AddOutput (0, finalTarget);
    //
    // move.AddInput (0, debugOutput);
    // move.AddOutput (0, finalTarget);
    //
    // present.AddInput (0, finalTarget);
}


TEST_F (HeadlessGoogleTestEnvironment, CompileTest)
{
    Device&      device      = GetDevice ();
    Queue&       queue       = GetGraphicsQueue ();
    CommandPool& commandPool = GetCommandPool ();

    RenderGraph graph (device, commandPool);
    graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 3),
                                                 ShaderPipeline::CreateShared (device, std::vector<std::filesystem::path> {
                                                                                           ShadersFolder / "test.vert",
                                                                                           ShadersFolder / "test.frag",
                                                                                       })));
}

template<typename DestinationType, typename SourceType>
DestinationType& DynamicRefCast (std::reference_wrapper<SourceType>& source)
{
    return dynamic_cast<DestinationType&> (source.get ());
}


TEST_F (HeadlessGoogleTestEnvironment, ShaderCompileTests)
{
    try {
        ShaderModule::CreateFromGLSLString (GetDevice (), ShaderModule::ShaderKind::Vertex, R"(
        #version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

    vec2 uvs[6] = vec2[] (
        vec2 (0.f, 0.f),
        vec2 (0.f, 1.f),
        vec2 (1.f,d 1.f),
        vec2 (1.f, 1.f),
        vec2 (0.f, 0.f),
        vec2 (1.f, 0.f)
    );

    vec2 positions[6] = vec2[] (
        vec2 (-1.f, -1.f),
        vec2 (-1.f, +1.f),
        vec2 (+1.f, +1.f),
        vec2 (+1.f, +1.f),
        vec2 (-1.f, -1.f),
        vec2 (+1.f, -1.f)
    );


    void main ()
    {
        gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
        textureCoords = uvs[gl_VertexIndex];
    }
)");
    } catch (ShaderCompileException& e) {
        std::cout << e.what () << std::endl;
    }
}


TEST_F (HeadlessGoogleTestEnvironment, RenderRedImage)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    GraphSettings s (device, graphicsQueue, commandPool, 3, 512, 512);
    RenderGraph   graph (device, commandPool);

    ImageResource& red = graph.CreateResource<WritableImageResource> ();

    auto sp = ShaderPipeline::Create (device);
    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )");

    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main () {
    outColor = vec4 (1, 0, 0, 1);
}
    )");

    Operation& redFillOperation = graph.AddOperation (RenderOperation::Create (DrawRecordableInfo::CreateShared (1, 6),
                                                                               std::move (sp)));

    graph.CreateOutputConnection (redFillOperation, 0, red);

    graph.Compile (s);

    graph.Submit (0);
    graph.Submit (1);
    graph.Submit (2);

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    CompareImages ("red", *dynamic_cast<WritableImageResource&> (red).images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    CompareImages ("red", *dynamic_cast<WritableImageResource&> (red).images[1]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    CompareImages ("red", *dynamic_cast<WritableImageResource&> (red).images[2]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HeadlessGoogleTestEnvironment, RenderGraphUseTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    GraphSettings s (device, graphicsQueue, commandPool, 4, 512, 512);
    RenderGraph   graph (device, commandPool);

    WritableImageResource& presented = graph.CreateResource<WritableImageResource> ();
    WritableImageResource& green     = graph.CreateResource<WritableImageResource> ();
    WritableImageResource& red       = graph.CreateResource<WritableImageResource> ();
    WritableImageResource& finalImg  = graph.CreateResource<WritableImageResource> ();

    Operation& dummyPass = graph.CreateOperation<RenderOperation> (DrawRecordableInfo::CreateShared (1, 3),
                                                                   ShaderPipeline::CreateShared (device, std::vector<std::filesystem::path> {
                                                                                                             ShadersFolder / "test.vert",
                                                                                                             ShadersFolder / "test.frag",
                                                                                                         }));

    Operation& secondPass = graph.CreateOperation<RenderOperation> (DrawRecordableInfo::CreateShared (1, 3),
                                                                    ShaderPipeline::CreateShared (device, std::vector<std::filesystem::path> {
                                                                                                              ShadersFolder / "fullscreenquad.vert",
                                                                                                              ShadersFolder / "fullscreenquad.frag",
                                                                                                          }));


    graph.CreateInputConnection<ImageInputBinding> (dummyPass, 0, green);
    graph.CreateOutputConnection (dummyPass, 0, presented);
    graph.CreateOutputConnection (dummyPass, 1, red);

    graph.CreateInputConnection<ImageInputBinding> (secondPass, 0, red);
    graph.CreateOutputConnection (secondPass, 0, finalImg);

    graph.Compile (s);

    Utils::DebugTimerLogger obs;
    {
        Utils::TimerScope _ (obs);
        for (uint32_t i = 0; i < 1; ++i) {
            graph.Submit (i);
        }
    }

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    TransitionImageLayout (device, graphicsQueue, commandPool, *green.images[0]->image.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *presented.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *red.images[0]->image.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout (device, graphicsQueue, commandPool, *finalImg.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    std::thread saveThreads[] = {
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *green.images[0]->image.image, ReferenceImagesFolder / "green.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *presented.images[0]->image.image, ReferenceImagesFolder / "presented.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *red.images[0]->image.image, ReferenceImagesFolder / "red.png"),
        SaveImageToFileAsync (device, graphicsQueue, commandPool, *finalImg.images[0]->image.image, ReferenceImagesFolder / "final.png"),
    };
    for (auto& t : saveThreads) {
        t.join ();
        std::cout << "saved" << std::endl;
    }

    ASSERT_TRUE (AreImagesEqual (device, graphicsQueue, commandPool, *presented.images[0]->image.image, ReferenceImagesFolder / "black.png"));
}


TEST_F (HiddenWindowGoogleTestEnvironment, SwapchainTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();


    GraphSettings s (device, graphicsQueue, commandPool, swapchain);
    RenderGraph   graph (device, commandPool);

    auto sp = ShaderPipeline::CreateShared (device);
    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 textureCoords;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    textureCoords = uvs[gl_VertexIndex];
}
    )");

    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outCopy;

void main () {
    vec4 result = vec4 (uv, 0, 1);
    outColor = result;
    outCopy = result;
}
    )");

    ImageResource& presentedCopy = graph.CreateResource<WritableImageResource> ();
    ImageResource& presented     = graph.CreateResource<SwapchainImageResource> (swapchain);

    Operation& redFillOperation = graph.CreateOperation<RenderOperation> (DrawRecordableInfo::CreateShared (1, 6), sp);

    graph.CreateOutputConnection (redFillOperation, 0, presented);
    graph.CreateOutputConnection (redFillOperation, 1, presentedCopy);

    graph.Compile (s);

    BlockingGraphRenderer renderer (graph, swapchain);
    window->DoEventLoop (renderer.GetCountLimitedDrawCallback (10));
}


TEST_F (HiddenWindowGoogleTestEnvironment, VertexAndIndexBufferTest)
{
    Device&       device        = GetDevice ();
    CommandPool&  commandPool   = GetCommandPool ();
    Queue&        graphicsQueue = GetGraphicsQueue ();
    Swapchain&    swapchain     = GetSwapchain ();
    GraphSettings s (device, graphicsQueue, commandPool, swapchain);
    RenderGraph   graph (device, commandPool);

    auto sp = ShaderPipeline::Create (device);
    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in float asd;

layout (location = 0) out vec2 textureCoords;
layout (location = 1) out float asdout;


void main() {
    gl_Position = vec4 (position, 0.0, 1.0);
    textureCoords = uv;
    asdout = asd;
}
    )");

    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 1) in float asdout;
layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outCopy;

void main () {
    vec4 result = vec4 (vec3 (uv, 0.f), 1);
    outColor = result;
    outCopy = result;
}
    )");

    WritableImageResource&  presentedCopy = graph.CreateResource<WritableImageResource> ();
    SwapchainImageResource& presented     = graph.CreateResource<SwapchainImageResource> (swapchain);

    struct Vert {
        glm::vec2 position;
        glm::vec2 uv;
        float     asd;
    };

    VertexBufferTransferable<Vert> vbb (device, graphicsQueue, commandPool, 4, {ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Float});
    vbb = std::vector<Vert> {
        {glm::vec2 (-1.f, -1.f), glm::vec2 (0.f, 0.f), 0.1f},
        {glm::vec2 (-1.f, +1.f), glm::vec2 (0.f, 1.f), 0.2f},
        {glm::vec2 (+1.f, +1.f), glm::vec2 (1.f, 1.f), 0.3f},
        {glm::vec2 (+1.f, -1.f), glm::vec2 (1.f, 0.f), 0.6f},
    };
    vbb.Flush ();

    IndexBufferTransferable ib (device, graphicsQueue, commandPool, 6);
    ib.data = {0, 1, 2, 0, 3, 2};
    ib.Flush ();

    RenderOperation& redFillOperation = graph.CreateOperation<RenderOperation> (DrawRecordableInfo::CreateShared (1, vbb.data.size (), vbb.buffer.GetBufferToBind (), vbb.info.bindings, vbb.info.attributes, ib.data.size (), ib.buffer.GetBufferToBind ()),
                                                                                std::move (sp));

    graph.CreateOutputConnection (redFillOperation, 0, presented);
    graph.CreateOutputConnection (redFillOperation, 1, presentedCopy);

    graph.Compile (s);

    BlockingGraphRenderer renderer (graph, swapchain);
    window->DoEventLoop (renderer.GetCountLimitedDrawCallback (10));

    CompareImages ("uv", *presentedCopy.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}


TEST_F (HiddenWindowGoogleTestEnvironment, BasicUniformBufferTest)
{
    Device&      device        = GetDevice ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();
    Swapchain&   swapchain     = GetSwapchain ();

    GraphSettings s (device, graphicsQueue, commandPool, swapchain);
    RenderGraph   graph (device, commandPool);

    auto sp = ShaderPipeline::Create (device);
    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform Time {
    float time;
} time;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in float asd;

layout (location = 0) out vec2 textureCoords;
layout (location = 1) out float asdout;


void main() {
    gl_Position = vec4 (position + vec2 (time.time), 0.0, 1.0);
    textureCoords = uv;
    asdout = asd;
}
    )");

    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform Time {
    float time;
} time;

layout (location = 1) in float asdout;
layout (location = 0) in vec2 uv;

layout (location = 2) out vec4 presented;
layout (location = 0) out vec4 copy[2];

void main () {
    vec4 result = vec4 (vec3 (uv, 0.f), 1);
    presented = result;
    copy[0] = result;
    copy[1] = result;
}
    )");

    SwapchainImageResource& presented     = graph.CreateResource<SwapchainImageResource> (swapchain);
    WritableImageResource&  presentedCopy = graph.CreateResource<WritableImageResource> (2);
    UniformBlockResource&   unif          = graph.CreateResource<UniformBlockResource> (4);

    struct Vert {
        glm::vec2 position;
        glm::vec2 uv;
        float     asd;
    };

    VertexBufferTransferable<Vert> vbb (device, graphicsQueue, commandPool, 4, {ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Float});
    vbb = std::vector<Vert> {
        {glm::vec2 (-1.f, -1.f), glm::vec2 (0.f, 0.f), 0.1f},
        {glm::vec2 (-1.f, +1.f), glm::vec2 (0.f, 1.f), 0.2f},
        {glm::vec2 (+1.f, +1.f), glm::vec2 (1.f, 1.f), 0.3f},
        {glm::vec2 (+1.f, -1.f), glm::vec2 (1.f, 0.f), 0.6f},
    };
    vbb.Flush ();

    IndexBufferTransferable ib (device, graphicsQueue, commandPool, 6);
    ib.data = {0, 1, 2, 0, 3, 2};
    ib.Flush ();

    Operation& redFillOperation = graph.CreateOperation<RenderOperation> (DrawRecordableInfo::CreateShared (1, vbb, ib), std::move (sp));

    graph.CreateInputConnection<UniformInputBinding> (redFillOperation, 0, unif);
    graph.CreateOutputConnection (redFillOperation, 0, presentedCopy);
    graph.CreateOutputConnection (redFillOperation, 2, presented);

    graph.Compile (s);

    BlockingGraphRenderer renderer (graph, swapchain);

    renderer.preSubmitEvent += [&] (uint32_t frameIndex, uint64_t) {
        float time = 0.5f;
        unif.GetMapping (frameIndex).Copy (time);
    };

    window->DoEventLoop (renderer.GetCountLimitedDrawCallback (10));

    CompareImages ("uvoffset", *presentedCopy.images[0]->image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

/*
#include "Cache.hpp"
#include <utility>

namespace GraphV2 {

class Node;

class OneWayConnection : public std::enable_shared_from_this<OneWayConnection> {
public:
    std::weak_ptr<Node> from;
    std::weak_ptr<Node> to;

    USING_CREATE (OneWayConnection);

    OneWayConnection (std::shared_ptr<Node>& from, std::shared_ptr<Node>& to)
        : from (from)
        , to (to)
    {
        ASSERT (IsValidConnection ());
    }

    virtual bool IsValidConnection () const { return true; }
};


class Node : public std::enable_shared_from_this<Node> {
public:
    USING_CREATE (Node);
    friend class Graph;

private:
    using NodeList       = std::vector<Node::W>;
    using ConnectionList = std::vector<OneWayConnection::W>;

    ConnectionList  connections;
    Cache<NodeList> pointingHere;
    Cache<NodeList> pointingTo;

    NodeList Node::GetNodesConnectedFromThis ()
    {
        auto thisShared = shared_from_this ();

        NodeList result;
        for (auto& c : connections) {
            if (auto cs = c.lock ()) {
                if (thisShared == cs->from.lock ()) {
                    result.push_back (cs->to);
                }
            }
        }
        return result;
    }

    NodeList Node::GetNodesConnectedToThis ()
    {
        auto thisShared = shared_from_this ();

        NodeList result;
        for (auto& c : connections) {
            if (auto cs = c.lock ()) {
                if (thisShared == cs->to.lock ()) {
                    result.push_back (cs->from);
                }
            }
        }
        return result;
    }


public:
    Node ()
        : pointingHere (std::bind (&Node::GetNodesConnectedToThis, this))
        , pointingTo (std::bind (&Node::GetNodesConnectedFromThis, this))
    {
    }

    virtual ~Node () = default;

    const NodeList& GetInputs () { return pointingHere; }
    const NodeList& GetOutputs () { return pointingTo; }

    virtual bool operator== (const Node& other) const { return false; }
    virtual bool CanConnect (const Node::P& other) const { return true; }

public:
    // either way
    void CreateOutputConnection (OneWayConnection::P conn)
    {
        connections.push_back (conn);

        pointingHere.Invalidate ();
        pointingTo.Invalidate ();
    }
};


class Graph {
private:
    std::set<Node::P>             nodes;
    std::set<OneWayConnection::P> connections;

public:
    USING_CREATE (Graph);

    template<typename ConnectionType, typename... ARGS>
    std::shared_ptr<ConnectionType> CreateOutputConnection (Node::P& from, Node::P& to, ARGS&&... args)
    {
        if (ERROR (!to->CanConnect (from))) {
            return nullptr;
        }

        std::shared_ptr<ConnectionType> asd = std::make_shared<ConnectionType> (from, to, std::forward<ARGS> (args)...);

        if (ERROR (!asd->IsValidConnection ())) {
            return nullptr;
        }

        from->CreateOutputConnection (asd);
        to->CreateOutputConnection (asd);
        connections.insert (asd);
        return asd;
    }

    template<typename NodeType, typename... ARGS>
    std::shared_ptr<NodeType> AddNode (ARGS&&... args)
    {
        std::shared_ptr<NodeType> asd = std::make_shared<NodeType> (std::forward<ARGS> (args)...);

        for (Node::P n : nodes) {
            if (*n == *asd) {
                return nullptr;
            }
        }

        nodes.insert (asd);
        return asd;
    }
};


class ResourceNode : public Node {
public:
    int i = 2;

    virtual bool CanConnect (const Node::P& other) const
    {
        // cannot connect to itself
        return std::dynamic_pointer_cast<ResourceNode> (other) == nullptr;
    }
};


class OperationNode : public Node {
public:
    int i = 3;

    virtual bool CanConnect (const Node::P& other) const
    {
        // cannot connect to itself
        return std::dynamic_pointer_cast<OperationNode> (other) == nullptr;
    }
};


class InputConnection : public OneWayConnection {
public:
    InputConnection (Node::P& from, Node::P& to, uint32_t)
        : OneWayConnection (from, to)
    {
    }

    virtual bool IsValidConnection () const override
    {
        return std::dynamic_pointer_cast<ResourceNode> (from.lock ()) != nullptr &&
               std::dynamic_pointer_cast<OperationNode> (to.lock ()) != nullptr;
    }
};


class OutputConnection : public OneWayConnection {
public:
    OutputConnection (Node::P& from, Node::P& to, uint32_t)
        : OneWayConnection (from, to)
    {
    }

    virtual bool IsValidConnection () const override
    {
        return std::dynamic_pointer_cast<OperationNode> (from.lock ()) != nullptr &&
               std::dynamic_pointer_cast<ResourceNode> (to.lock ()) != nullptr;
    }
};

} // namespace GraphV2


TEST_F (HiddenWindowGoogleTestEnvironment, graphtestttt)
{
    using namespace GraphV2;

    GraphV2::Graph g;

    Node::P r00 = g.AddNode<ResourceNode> ();
    Node::P r01 = g.AddNode<ResourceNode> ();
    Node::P r02 = g.AddNode<ResourceNode> ();
    Node::P r03 = g.AddNode<ResourceNode> ();
    Node::P r04 = g.AddNode<ResourceNode> ();

    Node::P n1 = g.AddNode<OperationNode> ();

    g.CreateOutputConnection<InputConnection> (r00, n1, 1);
    g.CreateOutputConnection<InputConnection> (r01, n1, 1);
    g.CreateOutputConnection<InputConnection> (r02, n1, 1);
    g.CreateOutputConnection<InputConnection> (r03, n1, 1);
    g.CreateOutputConnection<InputConnection> (r04, n1, 1);

    ASSERT_EQ (5, n1->GetInputs ().size ());
}
*/
