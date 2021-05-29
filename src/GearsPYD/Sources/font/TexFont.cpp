#include ".\texfont.h"
#include "StdAfx.h"

TexFont::TexFont (std::string faceName)
{
    glTextureName = 0;

    HDC   hdc   = CreateCompatibleDC (NULL);
    HFONT hfont = CreateFont (32, 0, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS,          // nOutPrecision
                              CLIP_DEFAULT_PRECIS,         // nClipPrecision
                              DEFAULT_QUALITY,             // nQuality
                              DEFAULT_PITCH | FF_DONTCARE, // nPitchAndFamily
                              faceName.c_str ());          // lpszFacename
    SelectObject (hdc, hfont);

    this->faceName = faceName;

    texHeight = texWidth = 512;
    teximage             = new unsigned char[texHeight * texWidth];
    for (unsigned int u = 0; u < texHeight * texWidth; u++)
        teximage[u] = 0;

    aGlyphs                     = 160;
    glyphs                      = new Glyph[aGlyphs];
    nGlyphs                     = 0;
    int        nPreLoadedGlyphs = 136;
    GlyphInfo* glyphMetrics     = new GlyphInfo[nPreLoadedGlyphs];

    int startGlyph = 32;
    for (int x = 0; x < nPreLoadedGlyphs; x++) {
        MAT2 mat;
        mat.eM11.value = mat.eM22.value = 1;
        mat.eM12.value = mat.eM21.value = 0;
        mat.eM11.fract = mat.eM22.fract = 0;
        mat.eM12.fract = mat.eM21.fract = 0;
        GetGlyphOutline (hdc, startGlyph + x, GGO_METRICS, &(glyphMetrics + x)->gm, 0, NULL, &mat);
        (glyphMetrics + x)->charcode = startGlyph + x;
    }
    qsort (glyphMetrics, nPreLoadedGlyphs, sizeof (GlyphInfo), TexFont::glyphCompare);

    gap            = 1;
    px             = gap;
    py             = gap;
    maxHeightInRow = 0;
    int c          = 0;
    for (; c < nPreLoadedGlyphs; c++) {
        placeGlyph (glyphMetrics[c], hdc);
    }

    delete glyphMetrics;
    DeleteObject (hfont);
    DeleteDC (hdc);
}

void TexFont::placeGlyph (GlyphInfo& gi, HDC hdc)
{
    if (nGlyphs >= aGlyphs) {
        aGlyphs += 16;
        Glyph* newArray = new Glyph[aGlyphs];
        for (int d = 0; d < nGlyphs; d++)
            newArray[d] = glyphs[d];
        delete glyphs;
        glyphs = newArray;
    }
    if (px + gi.gm.gmBlackBoxX + gap < texWidth && py + gi.gm.gmBlackBoxY + gap < texHeight) {
        // GLYPHMETRICS glym;
        unsigned char glyphBitmap[64 * 64];
        MAT2          mat;
        mat.eM11.value = mat.eM22.value = 1;
        mat.eM12.value = mat.eM21.value = 0;
        mat.eM11.fract = mat.eM22.fract = 0;
        mat.eM12.fract = mat.eM21.fract = 0;
        DWORD err                       = GetGlyphOutline (hdc, gi.charcode, GGO_GRAY4_BITMAP, &gi.gm /*glym*/, 64 * 64, glyphBitmap, &mat);
#pragma warning(push)
#pragma warning(disable : 4244)
        glyphs[nGlyphs].uvLeft     = (px - 0.5) / (double)texWidth;
        glyphs[nGlyphs].uvRight    = (px + 0.5 + gi.gm.gmBlackBoxX) / (double)texWidth;
        glyphs[nGlyphs].uvTop      = (py - 0.5) / (double)texHeight;
        glyphs[nGlyphs].uvBottom   = (py + 0.5 + gi.gm.gmBlackBoxY) / (double)texHeight;
        glyphs[nGlyphs].quadLeft   = gi.gm.gmptGlyphOrigin.x;
        glyphs[nGlyphs].quadRight  = (signed)gi.gm.gmptGlyphOrigin.x + (signed)gi.gm.gmBlackBoxX;
        glyphs[nGlyphs].quadTop    = gi.gm.gmptGlyphOrigin.y;
        glyphs[nGlyphs].quadBottom = (signed)gi.gm.gmptGlyphOrigin.y - (signed)gi.gm.gmBlackBoxY;
        glyphs[nGlyphs].advance    = gi.gm.gmCellIncX;
#pragma warning(pop)

        int glymb = 0;
        for (unsigned int piny = py; piny < py + gi.gm.gmBlackBoxY; piny++) {
            for (unsigned int pinx = px; pinx < px + gi.gm.gmBlackBoxX; pinx++) {
                if (glyphBitmap[glymb] == 0x10)
                    teximage[pinx + piny * texWidth] = 255;
                else
                    teximage[pinx + piny * texWidth] = glyphBitmap[glymb] * 16;
                glymb++;
            }
            glymb = ((glymb + 3) / 4) * 4;
        }

        if (gi.charcode >= 32 && gi.charcode < 32 + 136)
            lut[gi.charcode - 32] = &glyphs[nGlyphs];
        else
            extraGlyphs[gi.charcode] = &glyphs[nGlyphs];
        nGlyphs++;

        px += gi.gm.gmBlackBoxX + gap;
        if (maxHeightInRow < gi.gm.gmBlackBoxY)
            maxHeightInRow = gi.gm.gmBlackBoxY;
    } else {
        px = gap;
        py += maxHeightInRow + gap;
        maxHeightInRow = 0;
        if (py + gi.gm.gmBlackBoxY + gap < texHeight)
            placeGlyph (gi, hdc);
        else {
            if (gi.charcode >= 32 && gi.charcode < 32 + 136)
                lut[gi.charcode - 32] = &glyphs[nGlyphs - 1];
            else
                extraGlyphs[gi.charcode] = &glyphs[nGlyphs - 1];
            return;
        }
    }
}

TexFont::~TexFont (void)
{
    delete glyphs;
    delete teximage;
}

int TexFont::glyphCompare (const void* a, const void* b)
{
    GlyphInfo* c1 = (GlyphInfo*)a;
    GlyphInfo* c2 = (GlyphInfo*)b;

    return c2->gm.gmBlackBoxY - c1->gm.gmBlackBoxY;
}

TexFont::Glyph* TexFont::addGlyph (int c)
{
    HDC   hdc   = CreateCompatibleDC (NULL);
    HFONT hfont = CreateFont (32, 0, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS,          // nOutPrecision
                              CLIP_DEFAULT_PRECIS,         // nClipPrecision
                              DEFAULT_QUALITY,             // nQuality
                              DEFAULT_PITCH | FF_DONTCARE, // nPitchAndFamily
                              faceName.c_str ());          // lpszFacename
    SelectObject (hdc, hfont);

    GlyphInfo gif;
    gif.charcode = c;
    MAT2 mat;
    mat.eM11.value = mat.eM22.value = 1;
    mat.eM12.value = mat.eM21.value = 0;
    mat.eM11.fract = mat.eM22.fract = 0;
    mat.eM12.fract = mat.eM21.fract = 0;
    GetGlyphOutline (hdc, c, GGO_METRICS, &gif.gm, 0, NULL, &mat);
    placeGlyph (gif, hdc);
    glDisposeTexture ();
    glCreateTexture ();
    return glyphs + nGlyphs - 1;
}

void TexFont::glRenderString (
    std::string  str,
    std::string  fontFace,
    bool         italic,
    bool         bold,
    bool         underline,
    bool         strikeout,
    unsigned int nChars,
    unsigned int mode)
{
    using namespace Gears::Math;
    ::glBindTexture (GL_TEXTURE_2D, glTextureName);

    if (italic) {
        static const double italicsTrafo[16] = {1.0, 0.0, 0.0, 0.0,
                                                0.4, 1.0, 0.0, 0.0,
                                                0.0, 0.0, 1.0, 0.0,
                                                0.0, 0.0, 0.0, 1.0};
        glMultMatrixd (italicsTrafo);
    }
    if (bold) {
        static const double boldTrafo[16] = {1.3, 0.0, 0.0, 0.0,
                                             0.0, 1.0, 0.0, 0.0,
                                             0.0, 0.0, 1.0, 0.0,
                                             0.0, 0.0, 0.0, 1.0};
        glMultMatrixd (boldTrafo);
    }

    glEnable (GL_TEXTURE_2D);
    if (!(mode & TEXFONT_MODE_CLOSE_ONLY))
        glPushMatrix ();
    float3 underlineStart, underlineEnd;
    Glyph* g = NULL;
    for (const char* p = str.c_str (); *p && nChars; p++) {
        nChars--;
        int c = *p;
        if (c == '\n') {
            glPopMatrix ();
            Glyph& h = *lut['j' - 32];
            glTranslatef (0.0f, -1.5f * (h.quadTop - h.quadBottom), 0.0f);
            glPushMatrix ();
            continue;
        }
        if (c >= 32 && c < 32 + 136)
            g = lut[c - 32];
        else
            g = extraGlyphs[c];
        if (g == 0x0) {
            g = addGlyph (c);
        }
        glBegin (GL_QUADS);
        glTexCoord2f (g->uvLeft, g->uvTop);
        glVertex2f (g->quadLeft, g->quadTop);
        glTexCoord2f (g->uvLeft, g->uvBottom);
        glVertex2f (g->quadLeft, g->quadBottom);
        glTexCoord2f (g->uvRight, g->uvBottom);
        glVertex2f (g->quadRight, g->quadBottom);
        glTexCoord2f (g->uvRight, g->uvTop);
        glVertex2f (g->quadRight, g->quadTop);
        glEnd ();
        if (p == str) {
            underlineStart = float3 (g->quadLeft, g->quadBottom, 0.0);
        }
        glTranslatef (bold ? (g->advance * 0.9f) : g->advance, 0.0f, 0.0f);
        underlineStart.x -= bold ? (g->advance * 0.9f) : g->advance;
    }
    if (!(mode & TEXFONT_MODE_OPEN_ONLY))
        glPopMatrix ();
    glDisable (GL_TEXTURE_2D);
    if (underline || strikeout) {
        if (g)
            underlineEnd = float3 (g->quadRight - (bold ? (g->advance * 0.9f) : g->advance), g->quadBottom, 0.0f);

        glLineWidth (1.0);
        glBegin (GL_LINES);
        if (underline) {
            glVertex2d (underlineStart.x, underlineStart.y - 1.5);
            glVertex2d (underlineEnd.x, underlineEnd.y - 1.5);
            //				glVertex2d(underlineEnd.x,	underlineEnd.y - 2.0);
            //				glVertex2d(underlineStart.x, underlineStart.y - 2.0);
        }
        if (strikeout) {
            if (italic) {
                underlineStart.x += 6.0f * 0.4f;
                underlineEnd.x += 6.0f * 0.4f;
            }
            glVertex2d (underlineStart.x, underlineStart.y + 7.5);
            glVertex2d (underlineEnd.x, underlineEnd.y + 7.5);
            //				glVertex2d(underlineEnd.x,	underlineEnd.y + 7.0);
            //				glVertex2d(underlineStart.x, underlineStart.y + 7.0);
        }
        glEnd ();
    }
}

void TexFont::glBindTexture ()
{
    if (glTextureName == 0) {
        glCreateTexture ();
        return;
    }
    ::glBindTexture (GL_TEXTURE_2D, glTextureName);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void TexFont::glCreateTexture ()
{
    glGenTextures (1, &glTextureName);

    ::glBindTexture (GL_TEXTURE_2D, glTextureName);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY4, texWidth, texHeight, 0,
    //	GL_LUMINANCE, GL_UNSIGNED_BYTE, teximage);
    //	gluBuild2DMipmaps(GL_TEXTURE_2D, 1, texWidth, texHeight, GL_ALPHA, GL_UNSIGNED_BYTE, teximage);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_INTENSITY8, texWidth, texHeight, 0,
                  GL_RED, GL_UNSIGNED_BYTE, teximage);
}

void TexFont::glDisposeTexture ()
{
    if (glTextureName != 0)
        glDeleteTextures (1, &glTextureName);
}

void TexFont::glRenderFontTexture ()
{
    glEnable (GL_TEXTURE_2D);
    glBegin (GL_QUADS);
    glTexCoord2f (0.0, 0.0);
    glVertex2f (0.0, 0.0);
    glTexCoord2f (1.0, 0.0);
    glVertex2f (1.0, 0.0);
    glTexCoord2f (1.0, 1.0);
    glVertex2f (1.0, 1.0);
    glTexCoord2f (0.0, 1.0);
    glVertex2f (0.0, 1.0);
    glEnd ();
    glDisable (GL_TEXTURE_2D);
}

bool TexFont::matches (std::string fontFace)
{
    return fontFace == this->faceName;
}

glm::vec3 TexFont::getTextExtent (std::string otext, std::string fontFace, bool italic, bool bold, unsigned int nChars)
{
    using namespace Gears::Math;

    float3 extents, multiExtents;
    extents      = float3::zero;
    multiExtents = float3::zero;
    for (const char* p = otext.c_str (); *p && nChars; p++) {
        nChars--;
        Glyph* g;
        int    c = *p;
        if (c == 'N') {
            Glyph& h = *lut['j' - 32];
            if (h.quadTop - h.quadBottom > extents.y)
                extents.y = h.quadTop - h.quadBottom;
            if (extents.z > h.quadBottom)
                extents.z = h.quadBottom;
            if (italic)
                extents.x += extents.y * 0.4f;
            if (bold)
                extents.x *= 1.3f;

            if (extents.x > multiExtents.x)
                multiExtents.x = extents.x;
            multiExtents.y += extents.y;
            multiExtents.z = extents.z * multiExtents.y / extents.y;
            extents        = float3::zero;
        }
        if (c >= 32 && c < 32 + 136)
            g = lut[c - 32];
        else
            g = extraGlyphs[c];
        if (g == 0x0) {
            g = addGlyph (c);
        }
        Glyph& h = *g;
        if (bold)
            extents.x += h.advance * 0.9f;
        else
            extents.x += h.advance;
        if (h.quadTop - h.quadBottom > extents.y)
            extents.y = h.quadTop - h.quadBottom;
        if (extents.z > h.quadBottom)
            extents.z = h.quadBottom;
    }
    Glyph& h = *lut['j' - 32];
    if (h.quadTop - h.quadBottom > extents.y)
        extents.y = h.quadTop - h.quadBottom;
    if (extents.z > h.quadBottom)
        extents.z = h.quadBottom;
    if (italic)
        extents.x += extents.y * 0.4f;
    if (bold)
        extents.x *= 1.3f;
    if (extents.x > multiExtents.x)
        multiExtents.x = extents.x;
    multiExtents.y += extents.y;

    return multiExtents;
}