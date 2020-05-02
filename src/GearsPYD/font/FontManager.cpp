#include "StdAfx.h"
#include ".\fontmanager.h"

FontManager::FontManager(void)
{
}

FontManager::~FontManager(void)
{
	std::set<TexFont*>::iterator a = fonts.begin();
	while(a != fonts.end())
	{
		delete *a;
		a++;
	}
}

/*TexFont* FontManager::loadFont(char* faceName, CDC* pdc, int startGlyph, int nGlyphs)
{
	std::set<TexFont*>::iterator a = fonts.begin();
	while(a != fonts.end())
	{
		if(strcmp((*a)->faceName, faceName) == 0)
		{
			(*a)->glBindTexture();
			return *a;
		}
		a++;
	}
	TexFont* ntf = new TexFont(faceName, pdc, startGlyph, nGlyphs);
	fonts.insert(ntf);
	ntf->glBindTexture();
	return ntf;
}
*/

TexFont* FontManager::loadFont(std::string fontFace)
{
	std::set<TexFont*>::iterator a = fonts.begin();
	while(a != fonts.end())
	{
		if((*a)->matches(fontFace))
		{
			(*a)->glBindTexture();
			return *a;
		}
		a++;
	}
	TexFont* ntf = new TexFont(fontFace);
	fonts.insert(ntf);
	ntf->glBindTexture();
	return ntf;
}

void FontManager::glRenderAllFontTextures()
{
	std::set<TexFont*>::iterator a = fonts.begin();
	while(a != fonts.end())
	{
		(*a)->glBindTexture();
		(*a)->glRenderFontTexture();
		glTranslated(1.2, 0.0, 0.0);
		a++;
	}
}