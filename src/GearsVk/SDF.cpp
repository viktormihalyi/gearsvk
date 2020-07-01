#include "SDF.hpp"

#include "msdfgen-ext.h"
#include "msdfgen.h"

#include "Assert.hpp"

#include <cstring>
#include <map>


using namespace msdfgen;


FontHandle* GetFont (const std::filesystem::path& fontFile)
{
    static FreetypeHandle*                    ft = nullptr;
    static std::map<std::string, FontHandle*> loadedFonts;

    if (ft == nullptr) {
        ft = initializeFreetype ();
    }
    if (ERROR (ft == nullptr)) {
        throw std::runtime_error ("failed to initialize freetype");
    }

    const std::string file = fontFile.u8string ();

    if (loadedFonts.find (file) != loadedFonts.end ()) {
        return loadedFonts[file];
    }

    FontHandle* font = loadFont (ft, fontFile.u8string ().c_str ());

    if (ERROR (font == nullptr)) {
        throw std::runtime_error ("failed to load font");
    }

    loadedFonts[fontFile.u8string ()] = font;

    return font;
}


static Shape LoadShape (const std::filesystem::path& fontFile, uint32_t unicode)
{
    FontHandle* font = GetFont (fontFile);

    if (ERROR (font == nullptr)) {
        throw std::runtime_error ("failed to load font");
    }

    Shape shape;
    ASSERT (loadGlyph (shape, font, unicode));

    shape.normalize ();

    edgeColoringSimple (shape, 3.0);

    return shape;
}


template<typename T, uint32_t N>
static std::vector<T> ToVector (const Bitmap<T, N>& bitmap)
{
    std::vector<T> result;

    const size_t elemCount = bitmap.width () * bitmap.height () * N;

    result.resize (elemCount);

    memcpy (result.data (), static_cast<const T*> (bitmap), elemCount * sizeof (float));

    return result;
}


std::vector<float> GetGlyphSDF32x32x1 (const std::filesystem::path& fontFile, uint32_t unicode)
{
    const Shape shape = LoadShape (fontFile, unicode);

    Bitmap<float, 1> msdf (32, 32);

    generateSDF (msdf, shape, 4.0, 1.0, Vector2 (4.0, 4.0));

    return ToVector<float, 1> (msdf);
}


std::vector<float> GetGlyphMDF16x16x3 (const std::filesystem::path& fontFile, uint32_t unicode)
{
    const Shape shape = LoadShape (fontFile, unicode);

    Bitmap<float, 3> msdf (16, 16);

    generateMSDF (msdf, shape, 4.0, 1.0, Vector2 (4.0, 4.0));

    return ToVector<float, 3> (msdf);
}
