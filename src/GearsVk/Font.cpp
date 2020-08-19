#include "Font.hpp"

#include "msdfgen-ext.h"
#include "msdfgen.h"

#include "Assert.hpp"

#include <cstring>
#include <functional>
#include <map>


static msdfgen::FontHandle* GetFont (const std::filesystem::path& fontFile)
{
    static msdfgen::FreetypeHandle*                    ft = nullptr;
    static std::map<std::string, msdfgen::FontHandle*> loadedFonts;

    if (ft == nullptr) {
        ft = msdfgen::initializeFreetype ();
    }
    if (GVK_ERROR (ft == nullptr)) {
        throw std::runtime_error ("failed to initialize freetype");
    }

    const std::string file = fontFile.u8string ();

    if (loadedFonts.find (file) != loadedFonts.end ()) {
        return loadedFonts[file];
    }

    msdfgen::FontHandle* font = msdfgen::loadFont (ft, fontFile.u8string ().c_str ());
    if (GVK_ERROR (font == nullptr)) {
        throw std::runtime_error ("failed to load font");
    }

    loadedFonts[fontFile.u8string ()] = font;

    return font;
}


struct Font::Impl {
    msdfgen::FontHandle* fontHandle;

    Impl (const std::filesystem::path& fontFile)
        : fontHandle (GetFont (fontFile))
    {
    }
};


Font::Font (const std::filesystem::path& fontFile)
    : impl (std::make_unique<Font::Impl> (fontFile))
    , fontFile (fontFile)
{
    msdfgen::FontMetrics met;
    msdfgen::getFontMetrics (met, impl->fontHandle);

    emSize             = met.emSize;
    ascenderY          = met.ascenderY;
    descenderY         = met.descenderY;
    lineHeight         = met.lineHeight;
    underlineY         = met.underlineY;
    underlineThickness = met.underlineThickness;
}


Font::~Font ()
{
}


static msdfgen::Shape LoadShape (msdfgen::FontHandle* font, uint32_t unicode)
{
    if (GVK_ERROR (font == nullptr)) {
        throw std::runtime_error ("failed to load font");
    }

    msdfgen::Shape shape;
    GVK_ASSERT (msdfgen::loadGlyph (shape, font, unicode));

    shape.normalize ();

    msdfgen::edgeColoringSimple (shape, 3.0);

    GVK_ASSERT (shape.validate ());

    shape.inverseYAxis = true;

    return shape;
}


void Font::GetFontWhitespaceWidth (double& spaceAdvance, double& tabAdvance) const
{
    const bool success = msdfgen::getFontWhitespaceWidth (spaceAdvance, tabAdvance, impl->fontHandle);
    GVK_ASSERT (success);
}


double Font::GetKerning (uint32_t unicode1, uint32_t unicode2) const
{
    double result = 0.0;

    const bool success = msdfgen::getKerning (result, impl->fontHandle, unicode1, unicode2);
    GVK_ASSERT (success);

    return result;
}


template<typename T, size_t N>
static std::vector<T> BitmapToVector (const msdfgen::Bitmap<T, N>& bitmap)
{
    std::vector<T> result;

    const size_t elemCount = bitmap.width () * bitmap.height () * N;

    result.resize (elemCount);

    memcpy (result.data (), static_cast<const T*> (bitmap), elemCount * sizeof (float));

    return result;
}

// from https://github.com/Chlumsky/msdfgen/blob/master/main.cpp#L739
static void GetAutoFrame (const msdfgen::Shape::Bounds& bounds, uint32_t width, uint32_t height, msdfgen::Vector2& scale, msdfgen::Vector2& translate)
{
    double l = bounds.l, b = bounds.b, r = bounds.r, t = bounds.t;
    double range = 1;

    msdfgen::Vector2 frame (width, height);

    l -= .5 * range, b -= .5 * range, r += .5 * range, t += .5 * range;
    if (l >= r || b >= t)
        l = 0, b = 0, r = 1, t = 1;
    if (frame.x <= 0 || frame.y <= 0)
        GVK_ASSERT (false); // Cannot fit the specified pixel range
    msdfgen::Vector2 dims (r - l, t - b);

    if (dims.x * frame.y < dims.y * frame.x) {
        translate.set (.5 * (frame.x / frame.y * dims.y - dims.x) - l, -b);
        scale = frame.y / dims.y;
    } else {
        translate.set (-l, .5 * (frame.y / frame.x * dims.x - dims.y) - b);
        scale = frame.x / dims.x;
    }
}

template<size_t Components>
using DistanceMapGeneratorFunc = std::function<void (
    const msdfgen::BitmapRef<float, Components>& output,
    const msdfgen::Shape&                        shape,
    double                                       range,
    const msdfgen::Vector2&                      scale,
    const msdfgen::Vector2&                      translate)>;


template<size_t Components>
static Font::GlyphData GetGlyph (msdfgen::FontHandle*                 fontHandle,
                                 const uint32_t                       width,
                                 const uint32_t                       height,
                                 const uint32_t                       unicode,
                                 DistanceMapGeneratorFunc<Components> bitmapGeneratorFunc)
{
    msdfgen::Shape shape = LoadShape (fontHandle, unicode);

    msdfgen::Vector2 translate;
    msdfgen::Vector2 scale;
    GetAutoFrame (shape.getBounds (), width, height, scale, translate);

    msdfgen::Bitmap<float, Components> bitmapData (width, height);

    static const double range = 4.0;

    bitmapGeneratorFunc (bitmapData, shape, range, scale, translate);

    Font::GlyphData result;
    result.scale       = {scale.x, scale.y};
    result.translation = {translate.x, translate.y};
    result.data        = BitmapToVector<float, Components> (bitmapData);
    result.width       = width;
    result.height      = height;
    return result;
}


Font::GlyphData Font::GetGlyphSDF (uint32_t width, uint32_t height, uint32_t unicode) const
{
    auto generator = [] (const msdfgen::BitmapRef<float, 1>& output,
                         const msdfgen::Shape&               shape,
                         double                              range,
                         const msdfgen::Vector2&             scale,
                         const msdfgen::Vector2&             translate) {
        msdfgen::generateSDF (output, shape, range, scale, translate);
    };

    return GetGlyph<1> (impl->fontHandle, width, height, unicode, generator);
}


Font::GlyphData Font::GetGlyphMDF (uint32_t width, uint32_t height, uint32_t unicode) const
{
    auto generator = [] (const msdfgen::BitmapRef<float, 3>& output,
                         const msdfgen::Shape&               shape,
                         double                              range,
                         const msdfgen::Vector2&             scale,
                         const msdfgen::Vector2&             translate) {
        msdfgen::generateMSDF (output, shape, range, scale, translate);
    };

    return GetGlyph<3> (impl->fontHandle, width, height, unicode, generator);
}


Font::GlyphData Font::GetGlyphMTDF (uint32_t width, uint32_t height, uint32_t unicode) const
{
    auto generator = [] (const msdfgen::BitmapRef<float, 4>& output,
                         const msdfgen::Shape&               shape,
                         double                              range,
                         const msdfgen::Vector2&             scale,
                         const msdfgen::Vector2&             translate) {
        msdfgen::generateMTSDF (output, shape, range, scale, translate);
    };

    return GetGlyph<4> (impl->fontHandle, width, height, unicode, generator);
}
