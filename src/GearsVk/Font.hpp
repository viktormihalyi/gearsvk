#ifndef GEARSVK_FONT_HPP
#define GEARSVK_FONT_HPP

#include <cstdint>
#include <filesystem>
#include <vector>

#include "GearsVkAPI.hpp"
#include "Ptr.hpp"

#include "glmlib.hpp"

USING_PTR (Font);
class GEARSVK_API Font {
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

    struct GlyphData {
        std::vector<float> data;
        uint32_t           width;
        uint32_t           height;
        glm::vec2          translation;
        glm::vec2          scale;
    };

    // usese msdfgen::generateSDF to generate a single channel bitmap
    GlyphData GetGlyphSDF (uint32_t width, uint32_t height, uint32_t unicode) const;

    // usese msdfgen::generateMDF to generate a multi (3) channel bitmap
    GlyphData GetGlyphMDF (uint32_t width, uint32_t height, uint32_t unicode) const;

    // usese msdfgen::generateMTDF to generate a multi (4) channel bitmap
    GlyphData GetGlyphMTDF (uint32_t width, uint32_t height, uint32_t unicode) const;

    void   GetFontWhitespaceWidth (double& spaceAdvance, double& tabAdvance) const;
    double GetKerning (uint32_t unicode1, uint32_t unicode2) const;
};


#endif
