#ifndef GEARSVK_FONT_HPP
#define GEARSVK_FONT_HPP

#include <cstdint>
#include <filesystem>
#include <map>
#include <vector>

#include "Assert.hpp"
#include "GearsVkAPI.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"

#include "glmlib.hpp"

USING_PTR (GlyphData);
struct GlyphData {
    USING_CREATE (GlyphData);

    std::vector<float> data;
    uint32_t           width;
    uint32_t           height;
    glm::vec2          translation;
    glm::vec2          scale;
    glm::vec2          aspectRatio;

    glm::mat4 GetTransforMatrix () const;
};


USING_PTR (Font);
class GEARSVK_API Font : public Noncopyable {
public:
    // hide font handle
    struct Impl;
    std::unique_ptr<Impl> impl;

    const std::filesystem::path fontFile;

    double emSize;
    double ascenderY, descenderY;
    double lineHeight;
    double underlineY, underlineThickness;

public:
    USING_CREATE (Font);
    Font (const std::filesystem::path& fontFile);
    ~Font ();

    // usese msdfgen::generateSDF to generate a single channel bitmap
    GlyphData GetGlyphSDF (uint32_t width, uint32_t height, uint32_t unicode) const;

    // usese msdfgen::generateMDF to generate a multi (3) channel bitmap
    GlyphData GetGlyphMDF (uint32_t width, uint32_t height, uint32_t unicode) const;

    // usese msdfgen::generateMTDF to generate a multi (4) channel bitmap
    GlyphData GetGlyphMTDF (uint32_t width, uint32_t height, uint32_t unicode) const;

    void   GetFontWhitespaceWidth (double& spaceAdvance, double& tabAdvance) const;
    double GetKerning (uint32_t unicode1, uint32_t unicode2) const;
};


USING_PTR (FontManager)
class FontManager {
    USING_CREATE (FontManager);

public:
    enum class Type {
        SDF,
        MDF,
        MTDF
    };

private:
    Font           font;
    const uint32_t width;
    const uint32_t height;
    const Type     distanceFieldType;

    std::map<uint32_t, GlyphData> loadedGlyphs;

private:
    GlyphData Retrieve (const uint32_t unicode)
    {
        const auto it = loadedGlyphs.find (unicode);

        if (it == loadedGlyphs.end ()) {
            GlyphData data;

            if (distanceFieldType == Type::SDF) {
                data = font.GetGlyphSDF (width, height, unicode);
            } else if (distanceFieldType == Type::MDF) {
                data = font.GetGlyphMDF (width, height, unicode);
            } else if (distanceFieldType == Type::MTDF) {
                data = font.GetGlyphMTDF (width, height, unicode);
            } else {
                GVK_ASSERT (false);
            }

            std::cout << "generated '" << static_cast<char> (unicode) << "', scale: " << data.scale << ", translation: " << data.translation << std::endl;

            loadedGlyphs.insert ({ unicode, data });
            return data;
        }

        return it->second;
    }

public:
    FontManager (const std::filesystem::path& fontFile, const uint32_t width, const uint32_t height, const Type distanceFieldType)
        : font (fontFile)
        , width (width)
        , height (height)
        , distanceFieldType (distanceFieldType)
    {
    }

    GlyphData GetGlyph (const uint32_t unicode)
    {
        return Retrieve (unicode);
    }

    double GetKerning (uint32_t unicode1, uint32_t unicode2) const
    {
        return font.GetKerning (unicode1, unicode2);
    }
};


#endif
