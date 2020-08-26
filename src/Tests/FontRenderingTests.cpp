#include "GoogleTestEnvironment.hpp"

#include "DrawRecordableInfo.hpp"
#include "Font.hpp"
#include "ImageData.hpp"
#include "RenderGraph.hpp"
#include "VulkanWrapper.hpp"

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


TEST_F (FontRenderingTests, MSDFGEN)
{
    DeviceExtra& device        = GetDeviceExtra ();
    CommandPool& commandPool   = GetCommandPool ();
    Queue&       graphicsQueue = GetGraphicsQueue ();

    RG::GraphSettings s (device, 3, 512, 512);
    RG::RenderGraph   graph;

    constexpr uint32_t glyphWidthHeight = 64;

    RG::WritableImageResourceP outputImage = graph.CreateResource<RG::WritableImageResource> ();
    RG::ReadOnlyImageResourceP glyphs      = graph.CreateResource<RG::ReadOnlyImageResource> (VK_FORMAT_R32_SFLOAT, glyphWidthHeight, glyphWidthHeight, 1, 512);

    auto sp = ShaderPipeline::CreateShared (device);

    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

layout (location = 2) in vec2 glyphPos;       // instanced
layout (location = 3) in vec2 glyphTranslate; // instanced
layout (location = 4) in vec2 glyphScale;     // instanced
layout (location = 5) in vec2 glyphAsp;       // instanced
layout (location = 6) in uint glyphIndex;     // instanced

layout (location = 0) out vec2 textureCoords;
layout (location = 1) out uint glyphIdx;

void main ()
{
    const vec2 vertexPos = (position * glyphScale + glyphTranslate) * 0.2f;
    gl_Position = vec4 (vertexPos + glyphPos, 0.0, 1.0);
    textureCoords = 0.5f + glyphAsp / 2.f * (uv * 2.f - 1.f);
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
        glm::vec2 glyphTranslate;
        glm::vec2 glyphScale;
        glm::vec2 glyphAsp;
        uint32_t  glyphIndex;
    };

    FontManager fm ("C:\\Windows\\Fonts\\arialbd.ttf", glyphWidthHeight, glyphWidthHeight, FontManager::Type::SDF);

    auto instanceBuffer = VertexBufferTransferable<InstVert>::CreateShared (device, 512, std::vector<VkFormat> { ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Vec2f, ShaderTypes::Uint }, VK_VERTEX_INPUT_RATE_INSTANCE);
    *instanceBuffer     = {
        { glm::vec2 (-0.3, 0.0), fm.GetGlyph ('g').translation, fm.GetGlyph ('g').scale, fm.GetGlyph ('g').aspectRatio, 'g' },
        { glm::vec2 (+0.0, 0.0), fm.GetGlyph ('T').translation, fm.GetGlyph ('T').scale, fm.GetGlyph ('T').aspectRatio, 'T' },
        { glm::vec2 (+0.4, 0.0), fm.GetGlyph ('e').translation, fm.GetGlyph ('e').scale, fm.GetGlyph ('e').aspectRatio, 'e' },
        { glm::vec2 (+0.8, 0.0), fm.GetGlyph ('h').translation, fm.GetGlyph ('h').scale, fm.GetGlyph ('h').aspectRatio, 'h' },
        { glm::vec2 (+0.6, 0.0), fm.GetGlyph ('l').translation, fm.GetGlyph ('l').scale, fm.GetGlyph ('l').aspectRatio, 'l' },
    };


    instanceBuffer->Flush ();

    VertexBufferList vbs;
    vbs.Add (vbb);
    vbs.Add (instanceBuffer);

    RG::OperationP renderOp = graph.CreateOperation<RG::RenderOperation> (DrawRecordableInfo::CreateShared (5, 4, vbs, 6, ib.buffer.GetBufferToBind ()), sp);

    graph.CreateInputConnection (*renderOp, *glyphs, RG::ImageInputBinding::Create (0, *glyphs));
    graph.CreateOutputConnection (*renderOp, 0, *outputImage);

    graph.Compile (s);

    for (uint32_t unicode = 'A'; unicode < 'Z'; ++unicode) {
        glyphs->CopyLayer (fm.GetGlyph (unicode).data, unicode);
    }
    for (uint32_t unicode = 'a'; unicode < 'z'; ++unicode) {
        glyphs->CopyLayer (fm.GetGlyph (unicode).data, unicode);
    }
    glyphs->CopyLayer (fm.GetGlyph ('!').data, '!');

    graph.Submit (0);

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    CompareImages2 ("G.png", ImageData (ReferenceImagesFolder / "G.png"), ImageData (GetDeviceExtra (), *outputImage->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
}
