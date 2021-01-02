#include "GoogleTestEnvironment.hpp"

#include "DrawRecordableInfo.hpp"
#include "Font.hpp"
#include "ImageData.hpp"
#include "RenderGraph.hpp"
#include "UniformReflection.hpp"
#include "VulkanWrapper.hpp"
#include "GraphSettings.hpp"
#include "Operation.hpp"
#include "Resource.hpp"

#include "glmlib.hpp"

using FontRenderingTests = HiddenWindowGoogleTestEnvironment;


void CompareImages2 (const std::string& name, const ImageData& referenceImage, const ImageData& actualImage)
{
    const bool isSame = referenceImage == actualImage;

    EXPECT_TRUE (isSame);

    if (!isSame) {
        actualImage.SaveTo (PROJECT_ROOT / "temp" / name);
    }
}
#if 0

TEST_F (FontRenderingTests, MSDFGEN)
{
    DeviceExtra& device        = GetDeviceExtra ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    RG::GraphSettings s (device, 3);
    RG::RenderGraph   graph;

    constexpr uint32_t glyphWidthHeight = 16;

    auto sp = ShaderPipeline::CreateShared (device);

    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

layout (location = 2) in vec2 glyphPos;       // instanced
layout (location = 3) in uint glyphIndex;     // instanced

layout (location = 0) out vec2 textureCoords;
layout (location = 1) out uint glyphIdx;

struct SingleGlyphData {
    vec2 translation;
    vec2 scale;
    vec2 aspectRatio;
};

layout (binding = 1) uniform GlyphData {
    SingleGlyphData glyph[512];
    uint            fontSizePx;
};

layout (binding = 2) uniform Screen {
    uint screenWidth;
    uint screenHeight;
};


void main ()
{
    const vec2 scaling = vec2 (fontSizePx, fontSizePx) / (vec2 (screenWidth, screenHeight) / 2.f);
    const vec2 vertexPos = (position * glyph[glyphIndex].scale + glyph[glyphIndex].translation) * scaling;
    gl_Position = vec4 (vertexPos + glyphPos, 0.0, 1.0);
    textureCoords = 0.5f + glyph[glyphIndex].aspectRatio / 2.f * (uv * 2.f - 1.f);
    glyphIdx = glyphIndex;
}
    )");

    // draw second glyph
    sp->SetFragmentShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 textureCoords;
layout (location = 1) flat in uint glyphIndex;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2DArray sampl;


float GetMedian (float r, float g, float b) {
    return max (min (r, g), min (max (r, g), b));
}

float GetOpacity (sampler2DArray glyphArray, vec2 textureCoordinates, uint glyphIdx)
{
    const vec2 pos       = textureCoordinates.xy;
    const vec3 sampled   = texture (sampl, vec3 (textureCoordinates, glyphIdx)).rgb;
    const ivec2 sz       = textureSize (sampl, 0).xy;
    const float dx       = dFdx (pos.x) * sz.x; 
    const float dy       = dFdy (pos.y) * sz.y;
    const float toPixels = 8.0 * inversesqrt (dx * dx + dy * dy);
    const float sigDist  = GetMedian (sampled.r, sampled.g, sampled.b);
    const float w        = fwidth (sigDist);
    const float opacity  = smoothstep (0.5 - w, 0.5 + w, sigDist);
    return opacity;
}

void main ()
{
    const float dist = texture (sampl, vec3 (textureCoords.x, textureCoords.y, glyphIndex)).r;
    if (dist > 0.5f) {
        outColor = vec4 (vec3 (1), 1.f);
    } else {
        outColor = vec4 (vec3 (0), 1.f);
    }
    const float opacity = smoothstep (0.4, 0.6, dist);
    outColor = vec4 (vec3 (0), 1.f - opacity);

    // outColor = vec4 (vec3 (0), 0.9f - GetOpacity (sampl, textureCoords, glyphIndex));
}
    )");

    struct Vert {
        glm::vec2 position;
        glm::vec2 uv;
    };

    auto vbb = VertexBufferTransferable<Vert>::CreateShared (device, 512, std::vector<VkFormat> { ShaderTypes::Vec2f, ShaderTypes::Vec2f }, VK_VERTEX_INPUT_RATE_VERTEX);

    *vbb = std::vector<Vert> {
        { glm::vec2 (0.0f, 0.0f), glm::vec2 (0.f, 0.f) },
        { glm::vec2 (0.0f, -1.0f), glm::vec2 (0.f, 1.f) },
        { glm::vec2 (1.0f, -1.0f), glm::vec2 (1.f, 1.f) },
        { glm::vec2 (1.0f, 0.0f), glm::vec2 (1.f, 0.f) },
    };
    vbb->Flush ();

    IndexBufferTransferable ib (device, 6);
    ib.data = { 0, 1, 2, 0, 3, 2 };
    ib.Flush ();

    struct InstVert {
        glm::vec2 glyphPos;
        uint32_t  glyphIndex;
    };

    FontManager fm ("C:\\Windows\\Fonts\\arialbd.ttf", glyphWidthHeight, glyphWidthHeight, FontManager::Type::SDF);

    auto instanceBuffer = VertexBufferTransferable<InstVert>::CreateShared (device, 512, std::vector<VkFormat> { ShaderTypes::Vec2f, ShaderTypes::Uint }, VK_VERTEX_INPUT_RATE_INSTANCE);

    VertexBufferList vbs;
    vbs.Add (vbb);
    vbs.Add (instanceBuffer);

    RG::RenderOperationP renderOp = graph.CreateOperation<RG::RenderOperation> (DrawRecordableInfo::CreateShared (5, 4, vbs, 6, ib.buffer.GetBufferToBind ()), sp);

    RG::WritableImageResourceP outputImage = graph.CreateResource<RG::WritableImageResource> (512, 512);
    RG::ReadOnlyImageResourceP glyphs      = graph.CreateResource<RG::ReadOnlyImageResource> (VK_FORMAT_R32_SFLOAT, glyphWidthHeight, glyphWidthHeight, 1, 512);

    graph.CreateInputConnection (*renderOp, *glyphs, RG::ImageInputBinding::Create (0, *glyphs));
    graph.CreateOutputConnection (*renderOp, 0, *outputImage);

    RG::UniformReflection refl (graph);

    refl[renderOp][ShaderKind::Vertex]["GlyphData"]["fontSizePx"] = static_cast<uint32_t> (24);

    fm.glyphLoaded += [&] (uint32_t unicode) {
        GlyphData gdata                                                                  = fm.GetGlyph (unicode);
        refl[renderOp][ShaderKind::Vertex]["GlyphData"]["glyph"][unicode]["translation"] = gdata.translation;
        refl[renderOp][ShaderKind::Vertex]["GlyphData"]["glyph"][unicode]["scale"]       = gdata.scale;
        refl[renderOp][ShaderKind::Vertex]["GlyphData"]["glyph"][unicode]["aspectRatio"] = gdata.aspectRatio;
    };

    refl[renderOp][ShaderKind::Vertex]["Screen"]["screenWidth"]  = GetWindow ().GetWidth ();
    refl[renderOp][ShaderKind::Vertex]["Screen"]["screenHeight"] = GetWindow ().GetHeight ();
    GetWindow ().events.resized += [&] (uint32_t newWidth, uint32_t newHeight) {
        refl[renderOp][ShaderKind::Vertex]["Screen"]["screenWidth"]  = newWidth;
        refl[renderOp][ShaderKind::Vertex]["Screen"]["screenHeight"] = newHeight;
    };

    graph.Compile (s);

    *instanceBuffer = {
        { glm::vec2 (-0.3, 0.0), 'g' },
        { glm::vec2 (+0.0, 0.0), 'T' },
        { glm::vec2 (+0.4, 0.0), 'e' },
        { glm::vec2 (+0.8, 0.0), 'h' },
        { glm::vec2 (+0.6, 0.0), 'l' },
    };


    instanceBuffer->Flush ();


    for (uint32_t unicode = 'A'; unicode < 'Z'; ++unicode) {
        glyphs->CopyLayer (fm.GetGlyph (unicode).data, unicode);
    }
    for (uint32_t unicode = 'a'; unicode < 'z'; ++unicode) {
        glyphs->CopyLayer (fm.GetGlyph (unicode).data, unicode);
    }
    glyphs->CopyLayer (fm.GetGlyph ('!').data, '!');

    refl.Flush (0);
    graph.Submit (0);

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    CompareImages2 ("G.png", ImageData (ReferenceImagesFolder / "G.png"), ImageData (GetDeviceExtra (), *outputImage->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
}

#endif