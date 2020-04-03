#pragma once
#include <gl/gl.h>
#include <glm/glm.hpp>
#include <map>

#define TEXFONT_MODE_OPEN_AND_CLOSE 0
#define TEXFONT_MODE_OPEN_ONLY 1
#define TEXFONT_MODE_CLOSE_ONLY 2
#define TEXFONT_MODE_CONTINUE 3

class TexFont {
    friend class FontManager;

    class Glyph {
    public:
        float quadLeft;
        float quadRight;
        float quadTop;
        float quadBottom;
        float uvLeft;
        float uvRight;
        float uvTop;
        float uvBottom;
        float advance;
        int   charcode;
    };

    struct GlyphInfo {
        GLYPHMETRICS gm;
        int          charcode;
    };

    GLuint       glTextureName;
    std::string  faceName;
    unsigned int texWidth;
    unsigned int texHeight;
    int          maxAscent;
    int          maxDescent;
    int          nGlyphs;
    int          aGlyphs;

    unsigned int gap;
    unsigned int px;
    unsigned int py;
    unsigned int maxHeightInRow;

    unsigned char*        teximage;
    Glyph*                glyphs;
    Glyph*                lut[136]; //32-165
    std::map<int, Glyph*> extraGlyphs;

    void   placeGlyph (GlyphInfo& gi, HDC pdc);
    Glyph* addGlyph (int c);

public:
    void glCreateTexture ();
    void glDisposeTexture ();
    TexFont (std::string faceName);
    ~TexFont (void);

    static int glyphCompare (const void* a, const void* b);
    void       glBindTexture ();
    void       glRenderString (std::string str, std::string fontFace, bool italic, bool bold, bool underline, bool strikeout, unsigned int nChars = 0xefffffff, unsigned int mode = 0);
    void       glRenderFontTexture ();

    bool      matches (std::string fontFace);
    glm::vec3 getTextExtent (std::string otext, std::string fontFace, bool italic, bool bold, unsigned int nChars = 0xefffffff);

    GLuint getTextureId () { return glTextureName; }
};
