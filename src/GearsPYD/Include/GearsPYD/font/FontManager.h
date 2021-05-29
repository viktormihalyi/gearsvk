#pragma once

#include "TexFont.h"
#include <set>

class FontManager
{
	std::set<TexFont*> fonts;
public:
	FontManager(void);
	~FontManager(void);

	TexFont* loadFont(std::string fontFace);
	void glRenderAllFontTextures();
};
