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

    RG::WritableImageResourceP outputImage = graph.CreateResource<RG::WritableImageResource> ();
    RG::ReadOnlyImageResourceP glyphs      = graph.CreateResource<RG::ReadOnlyImageResource> (VK_FORMAT_R32G32B32A32_SFLOAT, 16, 16, 1, 512);

    auto sp = ShaderPipeline::CreateShared (device);

    sp->SetVertexShaderFromString (R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec2 glyphPos;   // instanced
layout (location = 3) in uint glyphIndex; // instanced

layout (location = 0) out vec2 textureCoords;
layout (location = 1) out uint glyphIdx;

void main() {
    gl_Position = vec4 (position + glyphPos, 0.0, 1.0);
    textureCoords = uv;
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

void main () {
    outColor = vec4 (vec3 (texture (sampl, vec3 (textureCoords.x, textureCoords.y, glyphIndex)).rgb), 1.f);

    outColor = vec4 (vec3 (0), 1.f - GetOpacity (sampl, textureCoords, glyphIndex));
}
    )");

    struct Vert {
        glm::vec2 position;
        glm::vec2 uv;
    };

    auto vbb = VertexBufferTransferable<Vert>::CreateShared (device, 512, std::vector<VkFormat> { ShaderTypes::Vec2f, ShaderTypes::Vec2f }, VK_VERTEX_INPUT_RATE_VERTEX);

    *vbb = std::vector<Vert> {
        { glm::vec2 (0.00f, 0.00f), glm::vec2 (0.f, 0.f) },
        { glm::vec2 (0.00f, 0.25f), glm::vec2 (0.f, 1.f) },
        { glm::vec2 (0.25f, 0.25f), glm::vec2 (1.f, 1.f) },
        { glm::vec2 (0.25f, 0.00f), glm::vec2 (1.f, 0.f) },
    };
    vbb->Flush ();

    IndexBufferTransferable ib (device, 6);
    ib.data = { 0, 1, 2, 0, 3, 2 };
    ib.Flush ();

    struct InstVert {
        glm::vec2 glyphPos;
        uint32_t  glyphIndex;
    };

    auto instanceBuffer = VertexBufferTransferable<InstVert>::CreateShared (device, 512, std::vector<VkFormat> { ShaderTypes::Vec2f, ShaderTypes::Uint }, VK_VERTEX_INPUT_RATE_INSTANCE);
    *instanceBuffer     = {
        { glm::vec2 (0.0, 0.0), 'H' },
        { glm::vec2 (0.3, 0.0), 'i' },
        { glm::vec2 (0.6, 0.0), '!' },
    };

    instanceBuffer->Flush ();

    VertexBufferList vbs;
    vbs.Add (vbb);
    vbs.Add (instanceBuffer);

    RG::OperationP renderOp = graph.CreateOperation<RG::RenderOperation> (DrawRecordableInfo::CreateShared (3, 4, vbs, 6, ib.buffer.GetBufferToBind ()), sp);

    graph.CreateInputConnection (*renderOp, *glyphs, RG::ImageInputBinding::Create (0, *glyphs));
    graph.CreateOutputConnection (*renderOp, 0, *outputImage);

    graph.Compile (s);

    // load font, populate texture array
    uint32_t   nextGlyphIndex = 0;
    const Font arial ("C:\\Windows\\Fonts\\arialbd.ttf");
    for (uint32_t unicode = 'A'; unicode < 'Z'; ++unicode) {
        const std::vector<float> glyphData = arial.GetGlyphMTDF (16, 16, unicode).data;
        glyphs->CopyLayer (glyphData, unicode);
    }
    for (uint32_t unicode = 'a'; unicode < 'z'; ++unicode) {
        const std::vector<float> glyphData = arial.GetGlyphMTDF (16, 16, unicode).data;
        glyphs->CopyLayer (glyphData, unicode);
    }
    auto dd = arial.GetGlyphMTDF (16, 16, '!').data;
    glyphs->CopyLayer (dd, '!');

    graph.Submit (0);

    vkQueueWaitIdle (graphicsQueue);
    vkDeviceWaitIdle (device);

    CompareImages2 ("G.png", ImageData (ReferenceImagesFolder / "G.png"), ImageData (GetDeviceExtra (), *outputImage->GetImages ()[0], 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
}
